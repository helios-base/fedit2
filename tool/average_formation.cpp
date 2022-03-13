
#include <rcsc/formation/formation_parser.h>
#include <rcsc/formation/formation.h>
#include <rcsc/formation/formation_dt.h>

#include <iostream>
#include <fstream>

using namespace rcsc;

class AverageConfGenerator {
private:
    Formation::Ptr M_base_formation;
    Formation::Ptr M_target_formation;
public:

private:
    Formation::Ptr openFormation( const char * filepath );

    bool openBaseFormation( const char * filepath );
    bool openTargetFormation( const char * filepath );

public:

    bool generate( const char * base_filepath,
                   const char * target_filepath );
};

/*-------------------------------------------------------------------*/
/*!

 */
Formation::Ptr
AverageConfGenerator::openFormation( const char * filepath )
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
bool
AverageConfGenerator::openBaseFormation( const char * filepath )
{
    M_base_formation = openFormation( filepath );

    if ( ! M_base_formation )
    {
        std::cerr << "Could not open the base formation ["
                  << filepath << "]" << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
AverageConfGenerator::openTargetFormation( const char * filepath )
{
    M_target_formation = openFormation( filepath );

    if ( ! M_target_formation )
    {
        std::cerr << "Could not open the target formation ["
                  << filepath << "]" << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
AverageConfGenerator::generate( const char * base_filepath,
                                const char * target_filepath )
{
    if ( ! openBaseFormation( base_filepath )
         || ! openTargetFormation( target_filepath ) )
    {
        return false;
    }

    FormationData::ConstPtr base_data = M_base_formation->toData();
    if ( ! base_data )
    {
        std::cerr << "Could not get the sample data of base formation."
                  << std::endl;
        return false;
    }

    Formation::Ptr average_formation( new FormationDT() );

    //
    // set role info
    //
    for ( int num = 1; num <= 11; ++num )
    {
        std::string role_name = M_base_formation->roleName( num );
        RoleType role_type  = M_base_formation->roleType( num );
        int paired_num = M_base_formation->pairedNumber( num );

        average_formation->setRole( num, role_name, role_type, paired_num );
    }

    //
    // create new data set
    //

    FormationData average_data;

    for ( const FormationData::Data & base : base_data->dataCont() )
    {
        FormationData::Data new_data;
        new_data.ball_ = base.ball_;

        M_target_formation->getPositions( base.ball_, new_data.players_ );

        for ( int num = 1; num <= 11; ++num )
        {
            new_data.players_[num-1] += base.players_[num-1];
            new_data.players_[num-1] *= 0.5;
        }

        average_data.addData( new_data );
    }


    //
    // train formation model
    //
    if ( ! average_formation->train( average_data ) )
    {
        return false;
    }

    //
    // print
    //
    average_formation->print( std::cout );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
static
void
usage( const char * prog )
{
    std::cerr << prog << " base.conf target.conf" << std::endl;
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

    AverageConfGenerator g;
    g.generate( argv[1], argv[2] );

    return 0;
}
