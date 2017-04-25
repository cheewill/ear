#include "catch.hpp"
#include <vector>


// vedctor
TEST_CASE( "vector<int> -> toString", "[toString][vector]" )
{
    std::vector<int> vv;
    REQUIRE( Catch::toString(vv) == "{  }" );
    vv.push_back( 42 );
    REQUIRE( Catch::toString(vv) == "{ 42 }" );
    vv.push_back( 250 );
    REQUIRE( Catch::toString(vv) == "{ 42, 250 }" );
}

TEST_CASE( "vector<string> -> toString", "[toString][vector]" )
{
    std::vector<std::string> vv;
    REQUIRE( Catch::toString(vv) == "{  }" );
    vv.push_back( "hello" );
    REQUIRE( Catch::toString(vv) == "{ \"hello\" }" );
    vv.push_back( "world" );
    REQUIRE( Catch::toString(vv) == "{ \"hello\", \"world\" }" );
}

namespace {
    /* Minimal Allocator */
    template<typename T>
    struct minimal_allocator {
        using value_type = T;
        using size_type = std::size_t;

        minimal_allocator() = default;
        template <typename U>
        minimal_allocator(const minimal_allocator<U>&) {}


        T *allocate( size_type n ) {
            return static_cast<T *>( ::operator new( n * sizeof(T) ) );
        }
        void deallocate( T *p, size_type /*n*/ ) {
            ::operator delete( static_cast<void *>(p) );
        }
        template<typename U>
        bool operator==( const minimal_allocator<U>& ) const { return true; }
        template<typename U>
        bool operator!=( const minimal_allocator<U>& ) const { return false; }
    };
}

TEST_CASE( "vector<int,allocator> -> toString", "[toString][vector,allocator][c++11][.]" ) {
    std::vector<int,minimal_allocator<int> > vv;
    REQUIRE( Catch::toString(vv) == "{  }" );
    vv.push_back( 42 );
    REQUIRE( Catch::toString(vv) == "{ 42 }" );
    vv.push_back( 250 );
    REQUIRE( Catch::toString(vv) == "{ 42, 250 }" );
}

TEST_CASE( "vec<vec<string,alloc>> -> toString", "[toString][vector,allocator][c++11][.]" ) {
    using inner = std::vector<std::string, minimal_allocator<std::string>>;
    using vector = std::vector<inner>;
    vector v;
    REQUIRE( Catch::toString(v) == "{  }" );
    v.push_back( inner { "hello" } );
    v.push_back( inner { "world" } );
    REQUIRE( Catch::toString(v) == "{ { \"hello\" }, { \"world\" } }" );
}
