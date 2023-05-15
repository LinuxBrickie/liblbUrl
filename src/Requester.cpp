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

#include <lb/url/Requester.h>

#include "HttpHandler.h"
#include "RequestHandler.h"

#include <atomic>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

#include <curl/curl.h>


namespace lb
{


namespace url
{


struct Requester::Private
{
  template<class T>
  using Lock = std::scoped_lock<T>;

  std::thread thread; //!< running and Response callback thread.
  std::mutex mutex;

  std::atomic<bool> running{ true };

  CURLM* multiHandle{ nullptr };

  using PendingRequests = std::queue< std::unique_ptr<RequestHandler> >;
  PendingRequests pendingRequests; //!< Protected by \a mutex

  using Requests = std::unordered_map< CURL*, std::unique_ptr<RequestHandler> >;
  Requests requests;


  Private()
    : multiHandle{ curl_multi_init() }
    , thread{}
  {
    if ( !multiHandle )
    {
      throw std::runtime_error( "Failed to create curl multi handle." );
    }
    thread = std::move( std::thread{ &Private::run, this } );
  }

  Private( const Private& ) = delete;
  Private& operator=( const Private& ) = delete;

  ~Private()
  {
    running = false;

    thread.join();

    curl_multi_cleanup( multiHandle );
  }

  void addRequest( http::Request request, http::Response::Callback response )
  {
    std::scoped_lock l{ mutex };

    pendingRequests.push( std::make_unique< HttpHandler >( std::move( request ), std::move( response ) ) );
  }

  bool addPendingRequests()
  {
    bool atLeastOneAdded{ false };

    // Assumes mutex is locked
    while ( !pendingRequests.empty() )
    {
      auto& pendingRequest{ pendingRequests.front() };
      if ( addPendingRequest( std::move( pendingRequest ) ) )
      {
        atLeastOneAdded = true;
      }
      else
      {
        // Could avoid sending this just now whilst the mutex is locked but that
        // can be done later, it ought not to be a common case.
        pendingRequest->respond( ResponseCode::eSendFailure );
      }
      pendingRequests.pop();
    }

    return atLeastOneAdded;
  }

  bool addPendingRequest( std::unique_ptr<RequestHandler> request )
  {
    try
    {
      const auto easyHandle{ request->getHandle() };

      curl_multi_add_handle( multiHandle, easyHandle );

      requests.emplace( std::piecewise_construct
                      , std::forward_as_tuple( easyHandle )
                      , std::forward_as_tuple( std::move( request ) ) );
    }
    catch( std::runtime_error e )
    {
      return false;
    }

    return true;
  }

  bool processInfo( CURL* easyHandle )
  {
    const auto I{ requests.find( easyHandle ) };
    if ( I == requests.end() )
    {
      return false;
    }

    auto& request{ I->second };
    //request->processInfo();
    request->respond( ResponseCode::eSuccess );

    curl_multi_remove_handle( multiHandle, easyHandle );

    requests.erase( I );
    return true;
  }

  void run()
  {
    static const int timeoutMilliseconds{ 500 };

    // We don't use this for anything yet but iot's a required argument for
    // curl_multi_perform.)
    int numHandlesRunning{ 0 };

    while ( running )
    {
      // 1. Add any new requests and call curl_multi_perform to ensure they get
      //    started.
      if ( addPendingRequests() )
      {
        //std::cout << "  Perform..." << std::endl;
        curl_multi_perform( multiHandle, &numHandlesRunning );
        //std::cout << "  " << numHandlesRunning << " still running" << std::endl;
      }

      // 2. Call curl_multi_poll for fd activity, keep poll timeout small so new
      //    requests are not delayed in being added by step 1. If any file
      //    descriptors have activity then we call curl_mutli_perform to deal
      //    with any data they may have.
      int numActiveFDs;
      const auto pollRC{ curl_multi_poll( multiHandle, nullptr, 0, timeoutMilliseconds, &numActiveFDs ) };
      //std::cout << numActiveFDs << " FDs" << std::endl;
      switch ( pollRC )
      {
      case CURLM_OK:
        if ( numActiveFDs > 0 )
        {
          curl_multi_perform( multiHandle, &numHandlesRunning );
        }
        break;
      default:
        std::cerr << "curl_multi_poll error: " << pollRC;
        continue;
      }

      // 3. Call curl_multi_info_read to monitor for changes (rather than rely
      //    on the curl_multi_perform info since we are only going to call that
      //    when new requests are added).
      read();
    }


    // Abort any requests that are still not complete.
    for ( auto& request : requests )
    {
      request.second->respond( ResponseCode::eAborted );
    }
  }

  void read()
  {
      CURLMsg* m{ nullptr };
      do
      {
        int msgq = 0;
        m = curl_multi_info_read( multiHandle, &msgq );
        //std::cout << "  " << msgq << " messages left in queue" << std::endl;
        if ( m && (m->msg == CURLMSG_DONE) )
        {
          CURL*const e{ m->easy_handle };
          if ( !processInfo( e ) )
          {
            std::cerr << "Read info for unknown curl easy handle" << std::endl;
          }
        }
      } while( m );
  }
};

Requester::Requester()
    : d{ new Private() }
{
}

Requester::~Requester() = default;

void Requester::makeRequest( http::Request request, http::Response::Callback response )
{
  d->addRequest( std::move( request ), std::move( response ) );
}

static struct GlobalSetup
{
  GlobalSetup()
  {
    if ( curl_global_init( CURL_GLOBAL_ALL ) == 0 )
    {
      success = true;
    }
  }

  ~GlobalSetup()
  {
    curl_global_cleanup();
  }

  bool success = false;

} globalSetup;


// static
bool Requester::wasGlobalInitSuccessful()
{
  return globalSetup.success;
}

// static
std::string Requester::getVersion()
{
  return curl_version();
}


} // End of namespace url


} // End of namespace lb
