
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
    Formation::Ptr ptr = FormationParser::parse( filepath );
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

    Formation::Ptr f = FormationParser::parse( filepath );

    if ( ! f )
    {
        std::cerr << "Could not open the formation " << filepath << std::endl;
        return 1;
    }

    f->print( std::cout );

    return 0;
}
