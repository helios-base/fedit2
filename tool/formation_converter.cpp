
#include <rcsc/formation/formation_parser.h>
#include <rcsc/formation/formation.h>
#include <rcsc/formation/formation_dt.h>

#include <iostream>
#include <fstream>

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
Formation::Ptr
openFormation( const char * filepath )
{
    FormationParser::Ptr parser = FormationParser::create( filepath );

    if ( ! parser )
    {
        std::cerr << "(openFormation) Could not create a parser for [" << filepath << "]" << std::endl;
        return Formation::Ptr();
    }

    std::ifstream fin( filepath );
    if ( ! fin )
    {
        return Formation::Ptr();
    }

    Formation::Ptr ptr = parser->parse( fin );

    return ptr;

}

/*-------------------------------------------------------------------*/
/*!

 */
static
void
usage( const char * prog )
{
    std::cerr << prog << " FORMATION.CONF" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
main( int argc, char ** argv )
{
    if ( argc < 2 )
    {
        usage( argv[0] );
        return 1;
    }

    std::string filepath = argv[1];

    Formation::Ptr f = openFormation( filepath.c_str() );

    if ( ! f )
    {
        std::cerr << "Could not open the formation " << filepath << std::endl;
        return 1;
    }

    f->print( std::cout );

    return 0;
}
