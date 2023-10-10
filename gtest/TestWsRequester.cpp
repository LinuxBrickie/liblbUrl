/*
    Copyright (C) 2023  Paul Fotheringham (LinuxBrickie)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "TestWsRequester.h"

#include <gtest/gtest.h>

#include <future>
#include <thread>

#include <lb/url/Requester.h>

#include "ServerList.h"

const std::string clientCloseReason{ "Client initiating close" };
const std::string serverCloseReason{ "Server initiating close" };


const std::unordered_map<std::string, ExpectedResponse> WSExpectedMockResponses
{
  {
    "/test/url/ws/invalid",
    {
      lb::url::ResponseCode::eFailure,
    }
  },
  {
    "/test/url/ws/hello",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello1",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello2",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello3",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello4",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } } }
    }
  },
  {
    "/test/url/ws/hello/name",
    {
      lb::url::ResponseCode::eSuccess,
      { { { "Hello world!" }, { "Hi there!" } }
      , { { "What's your name?" }, { "Paul" } } }
    }
  },
  {
    "/test/url/ws/hello/ping",
    {
      lb::url::ResponseCode::eSuccess,
      { { ping, "" } }
    }
  },
  {
    "/test/url/ws/goodbye",
    {
      lb::url::ResponseCode::eSuccess,
      { { sendControlCloseMessage, serverCloseReason } }
    }
  }
};

struct ChallengeConnection
{
  ChallengeConnection( lb::url::Requester& requester
                     , const std::string& url
                     , const ExpectedResponse& expectedResponse )
    : requester{ requester }
    , url{ url }
    , expectedResponse{ expectedResponse }
  {
  }

  struct ActualResponse
  {
    std::optional<lb::url::ws::DataOpCode> dataOpCode;
    std::optional<lb::url::ws::ControlOpCode> controlOpCode;
    std::string message;
  };

  void operator()( int port )
  {
    bool serverAskedToClose{ true };
    size_t numExpectedResponses{ expectedResponse.responseMessages.size() };
    if ( expectedResponse.responseMessages.empty()
      || expectedResponse.responseMessages.rbegin()->first != sendControlCloseMessage )
    {
      serverAskedToClose = false;
      ++numExpectedResponses;
    }

    using EstablishedPromise = std::promise< std::pair<lb::url::ResponseCode, lb::url::ws::Response> >;
    using DataRcvPromise     = std::promise<std::string>;
    using ControlRcvPromise  = std::promise< std::pair<lb::url::ws::ControlOpCode, std::string> >;

    EstablishedPromise ep;

    std::vector<std::promise<ActualResponse>> rps; // response promises
    rps.resize( numExpectedResponses );
    size_t responseIndex{ 0 };

    lb::url::ws::Receivers receivers
    {
      [&rps, &responseIndex, numExpectedResponses]( lb::url::ws::ConnectionID
                                                  , lb::url::ws::DataOpCode op
                                                  , std::string r ) // data receiver
        {
          ASSERT_LT( responseIndex, numExpectedResponses );
          ActualResponse response;
          response.dataOpCode = op;
          response.message = std::move( r );
          rps[responseIndex++].set_value( std::move( response ) );
        },
      [&rps, &responseIndex, numExpectedResponses]( lb::url::ws::ConnectionID
                                                  , lb::url::ws::ControlOpCode op
                                                  , std::string r ) // control receiver
        {
          ActualResponse response;
          response.controlOpCode = op;
          response.message = std::move( r );
          rps[responseIndex++].set_value( std::move( response ) );
        }
    };
    struct Canary
    {
      lb::url::ws::Receivers& receivers;
      ~Canary() { receivers.stopReceiving(); }
    } canary{ receivers };

    requester.makeRequest( {
                             "ws://" + hostColonPort( port ) + url,
                             receivers
                           }
                         , [&ep]( lb::url::ResponseCode rc, lb::url::ws::Response r )
                           {
                             ep.set_value( { rc, std::move( r ) } );
                           } );

    const auto establishedPair{ ep.get_future().get() };
    const lb::url::ResponseCode actualResponseCode{ establishedPair.first };
    const lb::url::ws::Response& actualResponse{ establishedPair.second };
    EXPECT_EQ( actualResponseCode, expectedResponse.responseCode );

    // Connection established
    if ( actualResponseCode == lb::url::ResponseCode::eSuccess )
    {
      size_t i = 0;
      for ( const auto&[challenge, expectedMessage] : expectedResponse.responseMessages )
      {
        std::optional<lb::url::ws::ControlOpCode> expectedControlOpCode;
        if ( challenge == ping )
        {
          auto sendResult{ actualResponse.senders.sendPing( "" ) };
          expectedControlOpCode = lb::url::ws::ControlOpCode::ePong;
          ASSERT_EQ( sendResult.get(), lb::url::ws::SendResult::eSuccess );
        }
        else
        {
          auto sendResult{ actualResponse.senders.sendData( lb::url::ws::DataOpCode::eText, challenge ) };
          ASSERT_EQ( sendResult.get(), lb::url::ws::SendResult::eSuccess );
          if ( challenge == sendControlCloseMessage )
          {
            expectedControlOpCode = lb::url::ws::ControlOpCode::eClose;
          }
        }

        // Block and wait for the next response.
        ActualResponse actualResponse{ rps[i].get_future().get() };

        if ( expectedControlOpCode.has_value() )
        {
          ASSERT_TRUE( actualResponse.controlOpCode.has_value() );
          EXPECT_EQ( *actualResponse.controlOpCode, expectedControlOpCode );
        }

        if ( serverAskedToClose )
        {
          // A data request that asks the server to close the connection should
          // always be the final challenge.

          // Following two are asserts because if they fail they indicate that
          // the test challenges have been set up incorrectly.
          ASSERT_EQ( i, expectedResponse.responseMessages.size() - 1 );
          ASSERT_EQ( serverCloseReason, expectedResponse.responseMessages.rbegin()->second );

          testClosure( actualResponse, serverAskedToClose );
        }
        else
        {
          EXPECT_EQ( actualResponse.message, expectedMessage );
        }

        ++i;
      }

      if ( !serverAskedToClose )
      {
        auto sendResult
        {
          actualResponse.senders.sendClose( lb::encoding::websocket::closestatus::toPayload(
                                              lb::encoding::websocket::closestatus::ProtocolCode::eNormal )
                                          , clientCloseReason )
        };
        EXPECT_EQ( sendResult.get(), lb::url::ws::SendResult::eSuccess );

        const auto actualResponse{ rps[i].get_future().get() };
        testClosure( actualResponse, serverAskedToClose );
      }

      // Should not be able to send anything now.
      auto sendResult = actualResponse.senders.sendData( lb::url::ws::DataOpCode::eText, "blah" );
      EXPECT_EQ( sendResult.get(), lb::url::ws::SendResult::eClosed );
      sendResult = actualResponse.senders.sendClose( lb::encoding::websocket::closestatus::toPayload(
                                                       lb::encoding::websocket::closestatus::ProtocolCode::eNormal ) );
      EXPECT_EQ( sendResult.get(), lb::url::ws::SendResult::eClosed );
    }
    else if ( actualResponseCode == lb::url::ResponseCode::eFailure )
    {
      // Should not be able to send anything.
      auto sendResult = actualResponse.senders.sendData( lb::url::ws::DataOpCode::eText, "blah" );
      EXPECT_EQ( sendResult.get(), lb::url::ws::SendResult::eNoImplementation );
      sendResult =
          actualResponse.senders.sendClose( lb::encoding::websocket::closestatus::toPayload(
                                              lb::encoding::websocket::closestatus::ProtocolCode::eNormal )
                                          , "Test finished" );
      EXPECT_EQ( sendResult.get(), lb::url::ws::SendResult::eNoImplementation );
    }
  }

  void testClosure( const ActualResponse& actualResponse
                  , bool serverAskedToClose )
  {
    ASSERT_TRUE( actualResponse.controlOpCode.has_value() );
    EXPECT_EQ( *actualResponse.controlOpCode, lb::url::ws::ControlOpCode::eClose );
    ASSERT_TRUE( actualResponse.message.size() >= 2 );
    const auto actualStatusCode
    {
      lb::encoding::websocket::closestatus::decodePayloadCode( actualResponse.message )
    };
    EXPECT_EQ( actualStatusCode
             , lb::encoding::websocket::closestatus::toPayload(
                 lb::encoding::websocket::closestatus::ProtocolCode::eNormal ) );
    EXPECT_EQ( actualResponse.message.substr( 2 )
             , serverAskedToClose ? serverCloseReason : clientCloseReason );
  }

  lb::url::Requester& requester;
  const std::string url;
  const ExpectedResponse expectedResponse;
};

TEST(Ws, Requester_Challenges_Serial)
{
  lb::url::Requester requester;

  const auto& serverConfigs = serverList.at( httpd::ServerType::eWebSocket );
  for ( const auto serverConfig : serverConfigs )
  {
    for ( const auto&[ url, challengeExpectedResponses ] : WSExpectedMockResponses )
    {
      ChallengeConnection{ requester, url, challengeExpectedResponses }( serverConfig.port );
    }
  }
}

TEST(Ws, Requester_Challenges_Parallel)
{
  lb::url::Requester requester;

  const auto& serverConfigs = serverList.at( httpd::ServerType::eWebSocket );
  for ( const auto& serverConfig : serverConfigs )
  {
    using Threads = std::vector<std::thread>;
    Threads threads{ WSExpectedMockResponses.size() };

    size_t c = 0;
    for ( const auto&[ url, challengeExpectedResponses ] : WSExpectedMockResponses )
    {
      threads[c] = std::move( std::thread{ ChallengeConnection{ requester
                                                              , url
                                                              , challengeExpectedResponses }
                                         , serverConfig.port } );
      ++c;
    }

    for ( auto& thread : threads )
    {
      thread.join();
    }
  }
}

void testRequesterDestruction( int port
                             , size_t pollTimeoutMilliseconds
                             , size_t closeTimeoutInMilliSeconds )
{
  lb::url::Requester requester{ { pollTimeoutMilliseconds } };
  std::promise<lb::url::ws::Response> connectionEstablishedPromise;
  requester.makeRequest( {
                           "ws://" + hostColonPort( port ) + "/test/url/ws/destruction",
                           lb::url::ws::Receivers
                           {
                             []( lb::url::ws::ConnectionID, lb::url::ws::DataOpCode, std::string ) // no-op data receiver
                             {
                             },
                             []( lb::url::ws::ConnectionID, lb::url::ws::ControlOpCode, std::string ) // no-op control receiver
                             {
                             }
                           },
                           closeTimeoutInMilliSeconds
                         }
                       , [ &connectionEstablishedPromise ]( lb::url::ResponseCode rc, lb::url::ws::Response r )
                         {
                           connectionEstablishedPromise.set_value( std::move( r ) );
                         } );

  lb::url::ws::Response actualResponse{ connectionEstablishedPromise.get_future().get() };
}

TEST(Ws, Requester_Destruction)
{
  const auto& serverConfigs = serverList.at( httpd::ServerType::eWebSocket );
  for ( const auto& serverConfig : serverConfigs )
  {
    // Start with a quick poll timeout and get successively longer until we reach
    // the default value. The first iteration will not have time to receive the
    // close confirmation from the server whereas the last should.

    // A close timeout shorter than the poll timeout should guarantee that the
    // close confirmation from the server will not be received, thereby testing
    // that the connection is forcibly shutdown correctly.
    testRequesterDestruction( serverConfig.port
                            , 200  // poll timeout
                            , 1 ); // close timeoue

    // A close timeout longer than the poll timeout and of a reasonable size
    // should guarantee that the close confirmation from the server will be
    // received.
    testRequesterDestruction( serverConfig.port
                            , 300     // poll timeout
                            , 1000 ); // close timeout

    // Test closing off the Receivers knowing that data will be sent back
    lb::url::ws::Receivers receivers
    {
      []( lb::url::ws::ConnectionID, lb::url::ws::DataOpCode, std::string r ) // data receiver
      {
      },
      []( lb::url::ws::ConnectionID, lb::url::ws::ControlOpCode, std::string r )
      {
      }
    };

    lb::url::ws::Request request
    {
      "ws://" + hostColonPort( serverConfig.port ) + "/test/url/ws/destruction/with-data",
      receivers
    };

    // Can do it now before we even make the request. This guarantees they were
    // closed off before any data arrives!
    receivers.stopReceiving();

    lb::url::Requester requester;
    std::promise<lb::url::ws::Response> connectionEstablishedPromise;
    requester.makeRequest( request
                         , [ &connectionEstablishedPromise ]( lb::url::ResponseCode rc, lb::url::ws::Response r )
                           {
                             connectionEstablishedPromise.set_value( std::move( r ) );
                           } );

    lb::url::ws::Response actualResponse{ connectionEstablishedPromise.get_future().get() };

    size_t i = 0;
    auto sendResult{ actualResponse.senders.sendData( lb::url::ws::DataOpCode::eText, "any old rubbish" ) };
    EXPECT_EQ( sendResult.get(), lb::url::ws::SendResult::eSuccess );

    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
  }
}
