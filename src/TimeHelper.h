/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef TIMEHELPER_H_G9PHEDZA
#define TIMEHELPER_H_G9PHEDZA

#include <ctime>
#include <cassert>
#include <chrono>

namespace lnils {
namespace time {

inline timespec now()
{
     timespec ts;
     int ret = clock_gettime( CLOCK_MONOTONIC_COARSE, &ts );
     if( ret != 0 ) {
          throw "omg why does clock_gettime fail?";
     }
     return ts;
}

inline timespec duration( const timespec& start, const timespec& end )
{
     timespec result;
     if( ( end.tv_nsec - start.tv_nsec ) < 0 ) {
          result.tv_sec = end.tv_sec - start.tv_sec - 1;
          result.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
     }
     else {
          result.tv_sec = end.tv_sec - start.tv_sec;
          result.tv_nsec = end.tv_nsec - start.tv_nsec;
     }
     return result;
}

enum Relation { LT, EQ, GT };

inline Relation cmp( const timespec& ts1, const timespec& ts2 )
{
     if( ts1.tv_sec < ts2.tv_sec ) {
          return LT;
     }
     else if( ts1.tv_sec > ts2.tv_sec ) {
          return GT;
     }
     else { /* ts1.tv_sec == ts2.tv_sec */
          if( ts1.tv_nsec < ts2.tv_nsec ) {
               return LT;
          }
          else if( ts1.tv_nsec > ts2.tv_nsec ) {
               return GT;
          }
          else { /* ts1.tv_nsec == ts2.tv_nsec */
               return EQ;
          }
     }
     assert( 0 );
}

inline timespec advance( const timespec& ts, const std::chrono::steady_clock::duration& duration )
{
     return {
          ts.tv_sec + std::chrono::duration_cast<std::chrono::seconds>( duration ).count(),
          ts.tv_nsec + std::chrono::duration_cast<std::chrono::nanoseconds>( duration ).count()
     };
}

inline long to_ms( const timespec& ts )
{
     return ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
}

}
}

#endif /* end of include guard: TIMEHELPER_H_G9PHEDZA */

