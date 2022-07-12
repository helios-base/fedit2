// -*-c++-*-

/*!
  \file feditor_data.cpp
  \brief formation editor data class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QDateTime>
#include <QFile>

#include "edit_data.h"

#include "options.h"

#include <rcsc/formation/formation_parser.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/math_util.h>

#include <fstream>
#include <iostream>

using namespace rcsc;

const double EditData::MAX_X = ServerParam::DEFAULT_PITCH_LENGTH * 0.5 + 2.0;
const double EditData::MAX_Y = ServerParam::DEFAULT_PITCH_WIDTH * 0.5 + 2.0;

namespace {

inline
Vector2D
round_coordinates( const double x,
                   const double y )
{
    return Vector2D( rint( bound( - EditData::MAX_X, x, EditData::MAX_X ) / FormationData::PRECISION ) * FormationData::PRECISION,
                     rint( bound( - EditData::MAX_Y, y, EditData::MAX_Y ) / FormationData::PRECISION ) * FormationData::PRECISION );
}

}


/*-------------------------------------------------------------------*/
/*!

 */
EditData::EditData()
    : M_filepath()
    , M_conf_changed( false )
    , M_current_index( -1 )
    , M_select_type( NO_SELECT )
    , M_select_index( 0 )
    , M_constraint_origin_index( -1 )
    , M_constraint_terminal_index( -1 )
    , M_constraint_terminal( Vector2D::INVALIDATED )
{
    init();
}

/*-------------------------------------------------------------------*/
/*!

 */
EditData::~EditData()
{
    //saveChanged();
    //std::cerr << "delete EditData" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::init()
{
    M_conf_changed = false;
    M_formation_data.reset();

    M_current_state.ball_.assign( 0.0, 0.0 );
    for ( size_t i = 0; i < M_current_state.players_.size(); ++i )
    {
        M_current_state.players_[i].assign( -3.0 * i + 3.0, -37.0 );
    }

    M_constraint_origin_index = -1;
    M_constraint_terminal_index = -1;
    M_constraint_terminal = Vector2D::INVALIDATED;

    M_select_type = NO_SELECT;
    M_select_index = 0;

    M_triangulation.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::createFormation( const QString & type_name )
{
    init();

    M_formation = Formation::create( type_name.toStdString() );
    if ( ! M_formation )
    {
        std::cerr << "(EditData::createFormation) Failed to create a formation. name=" << type_name.toStdString() << std::endl;
        return;
    }

    M_formation->setRole( 1, "Goalie", RoleType( RoleType::Goalie, RoleType::Center ), 0 );
    M_formation->setRole( 2, "CenterBack", RoleType( RoleType::Defender, RoleType::Left ), 3 );
    M_formation->setRole( 3, "CenterBack", RoleType( RoleType::Defender, RoleType::Right ), 2 );
    M_formation->setRole( 4, "SideBack", RoleType( RoleType::Defender, RoleType::Left ), 5 );
    M_formation->setRole( 5, "SideBack", RoleType( RoleType::Defender, RoleType::Right ), 4 );
    M_formation->setRole( 6, "DefensiveHalf", RoleType( RoleType::MidFielder, RoleType::Center ), 0 );
    M_formation->setRole( 7, "OffensiveHalf", RoleType( RoleType::MidFielder, RoleType::Left ), 8 );
    M_formation->setRole( 8, "OffensiveHalf", RoleType( RoleType::MidFielder, RoleType::Right ), 7 );
    M_formation->setRole( 9, "SideForward", RoleType( RoleType::Forward, RoleType::Left ), 10 );
    M_formation->setRole( 10, "SideForward", RoleType( RoleType::Forward, RoleType::Right ), 9 );
    M_formation->setRole( 11, "CenterForward", RoleType( RoleType::Forward, RoleType::Center ), 0 );

    M_formation_data = FormationData::Ptr( new FormationData() );
    if ( ! M_formation_data )
    {
        std::cerr << "(EditData::createFormation) Failed to create a formation data." << std::endl;
        return;
    }

    FormationData::Data data;
    data.ball_.assign( 0.0, 0.0 );
    data.players_.emplace_back( -50.0, 0.0 );
    data.players_.emplace_back( -20.0, -8.0 );
    data.players_.emplace_back( -20.0, 8.0 );
    data.players_.emplace_back( -18.0, -18.0 );
    data.players_.emplace_back( -18.0, 18.0 );
    data.players_.emplace_back( -15.0, 0.0 );
    data.players_.emplace_back( 0.0, -12.0 );
    data.players_.emplace_back( 0.0, 12.0 );
    data.players_.emplace_back( 10.0, -22.0 );
    data.players_.emplace_back( 10.0, 22.0 );
    data.players_.emplace_back( 10.0, 0.0 );

    M_formation_data->addData( data );

    train();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::backup( const QString & filepath )
{
    if ( filepath.isEmpty() )
    {
        return;
    }

    QString backup_filepath = filepath;

    if ( M_formation
         && ! M_formation->version().empty() )
    {
        backup_filepath += QString( "_" );
        backup_filepath += QString::fromStdString( M_formation->version() );
    }
    else if ( M_saved_datetime.isEmpty() )
    {
        backup_filepath += QString( "_" );
        backup_filepath += QDateTime::currentDateTime().toString( QString( "yyyyMMdd-hhmmss" ) );
        backup_filepath += QString( ".conf" );
    }
    else
    {
        backup_filepath += QString( "_" );
        backup_filepath += M_saved_datetime;
        //backup_filepath += QString( ".conf" );
    }

    if ( ! QFile::copy( filepath, backup_filepath ) )
    {
        std::cerr << "Failed to backup the file [" << filepath.toStdString()  << "]"
                  << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::openConf( const QString & filepath )
{
    FormationParser::Ptr parser = FormationParser::create( filepath.toStdString() );

    if ( ! parser )
    {
        std::cerr << "(EditData::openConf) Could not create the parser instance for [" << filepath.toStdString() << "]" << std::endl;
        return false;
    }

    std::cerr << "(EditData::openConf) parser type = " << parser->name() << std::endl;

    std::ifstream fin( filepath.toStdString().c_str() );
    if ( ! fin.is_open() )
    {
        std::cerr << "(EditData::openConf) Failed to open formation file [" << filepath.toStdString() << "]" << std::endl;
        return false;
    }

    M_formation = parser->parse( fin );
    if ( ! M_formation )
    {
        std::cerr << "(EditData::openConf) Failed to read a formation [" << filepath.toStdString() << "]" << std::endl;
        return false;
    }

    init();

    M_formation_data = M_formation->toData();
    if ( ! M_formation_data )
    {
        std::cerr << "(EditData::openConf) Failed to create a formation data [" << filepath.toStdString() << "]" << std::endl;
        return false;
    }

    M_filepath = filepath;

    updateTriangulation();
    updatePlayerPosition();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::openData( const QString & filepath )
{
    if ( ! M_formation )
    {
        std::cerr << "(openData) No formation! create a new one or open an exsiting one." << std::endl;
        return false;
    }

    M_triangulation.clear();

    std::ifstream fin( filepath.toStdString().c_str() );
    if ( ! fin )
    {
        std::cerr << "(openData) Could not open the file " << filepath.toStdString() << std::endl;
        return false;
    }

    M_formation_data = FormationData::Ptr( new FormationData() );
    if ( ! M_formation_data->read( fin ) )
    {
        std::cerr << "(openData) Could not read the file " << filepath.toStdString() << std::endl;
        return false;
    }

    M_current_index = -1;
    train();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::saveDataAs( const QString & filepath )
{
    if ( ! M_formation_data )
    {
        return false;
    }

    std::ofstream fout( filepath.toStdString().c_str() );
    if ( ! fout.is_open() )
    {
        fout.close();
        return false;
    }

    M_formation_data->print( fout );

    fout.flush();
    fout.close();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::openBackgroundConf( const QString & filepath )
{
    FormationParser::Ptr parser = FormationParser::create( filepath.toStdString() );

    if ( ! parser )
    {
        std::cerr << "Could not create the parser instance for [" << filepath.toStdString() << "]" << std::endl;
        return false;
    }

    std::ifstream fin( filepath.toStdString().c_str() );
    if ( ! fin.is_open() )
    {
        std::cerr << "(openBackgroundConf) Failed to open formation file [" << filepath.toStdString() << "]" << std::endl;
        return false;
    }

    M_background_formation = parser->parse( fin );
    if ( ! M_background_formation )
    {
        std::cerr << "(EditData::openBackgroundConf) Failed to create a background formation. [" << filepath.toStdString() << "]" << std::endl;
        return false;
    }

    FormationData::Ptr background_data = M_background_formation->toData();
    if ( background_data )
    {
        M_background_triangulation.clear();

        for ( const FormationData::Data & d : background_data->dataCont() )
        {
            M_background_triangulation.addPoint( d.ball_ );
        }

        // for ( const FormationData::Constraint & c : M_background_formation->data()->constraints() )
        // {
        //     M_background_triangulation.addConstraint( static_cast< size_t >( c.first->index_ ),
        //                                               static_cast< size_t >( c.second->index_ ) );
        // }

        M_background_triangulation.compute();
        //M_background_triangulation.updateHalfEdges();
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::saveConf()
{
    if ( M_filepath.isEmpty() )
    {
        return false;
    }

    return saveConfAs( M_filepath );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::saveConfAs( const QString & filepath )
{
    if ( ! M_formation )
    {
        return false;
    }

    M_saved_datetime = QDateTime::currentDateTime().toString( QString( "yyyyMMdd-hhmmss" ) );
    M_formation->setVersion( M_saved_datetime.toStdString() );

    std::ofstream fout( filepath.toStdString().c_str() );
    if ( ! fout.is_open() )
    {
        fout.close();
        return false;
    }

    if ( ! M_formation->print( fout ) )
    {
        return false;
    }

    fout.flush();
    fout.close();

    std::cerr << "Saved to  [" << filepath.toStdString() << "]" << std::endl;

    if ( Options::instance().autoBackup() )
    {
        //backup( M_filepath );
        QString backup_filepath = filepath;
        backup_filepath += QString( "_" );
        backup_filepath += QDateTime::currentDateTime().toString( QString( "yyyyMMdd-hhmmss" ) );
        if ( ! QFile::copy( filepath, backup_filepath ) )
        {
            std::cerr << "Failed to backup the file [" << filepath.toStdString()  << "]"
                      << std::endl;
        }
        else
        {
            std::cerr << "Copied to [" << backup_filepath.toStdString() << "]" << std::endl;
        }
    }

    M_conf_changed = false;
    M_filepath = filepath;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::updatePlayerPosition()
{
    if ( ! Options::instance().playerAutoMove() )
    {
        return;
    }

    if ( ! M_formation )
    {
        return;
    }

    M_formation->getPositions( M_current_state.ball_, M_current_state.players_ );

    for ( std::vector< Vector2D >::iterator it = M_current_state.players_.begin();
          it != M_current_state.players_.end();
          ++it )
    {
        *it = round_coordinates( it->x, it->y );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::updateTriangulation()
{
    if ( ! M_formation_data )
    {
        return;
    }

    M_triangulation.clear();

    for ( const FormationData::Data & d : M_formation_data->dataCont() )
    {
        M_triangulation.addPoint( d.ball_ );
    }

    // for ( const FormationData::Constraint & c : M_formation_data->constraints() )
    // {
    //     M_triangulation.addConstraint( static_cast< size_t >( c.first->index_ ),
    //                                    static_cast< size_t >( c.second->index_ ) );
    // }

    M_triangulation.compute();
    //M_triangulation.updateHalfEdges();

    std::cerr << "updateTriangulation"
        //               << "\n  vertices=" << M_triangulation.indexedVertices().size()
        //               << "\n  edges=" << M_triangulation.halfEdges().size()
              << "\n  edges=" << M_triangulation.edges().size()
              << "\n  triangles=" << M_triangulation.triangles().size()
              << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::updateRoleData( const int num,
                          const int paired_num,
                          const std::string & role_name )
{
    if ( ! M_formation )
    {
        return;
    }

    M_formation->setRoleName( num, role_name );
    M_formation->setPositionPair( num, paired_num );

    M_conf_changed = true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::updateRoleType( const int num,
                          const int type_index,
                          const int side_index )
{
    if ( ! M_formation )
    {
        return;
    }

    RoleType role_type = M_formation->roleType( num );
    role_type.setType( static_cast< RoleType::Type >( type_index ) );
    role_type.setSide( side_index == 1 ? RoleType::Left :
                       side_index == 2 ? RoleType::Right :
                       RoleType::Center );

    M_formation->setRoleType( num, role_type );

    M_conf_changed = true;
}

/*-------------------------------------------------------------------*/
/*!

 */
// void
// EditData::updateMarkerData( const int num,
//                             const bool marker,
//                             const bool setplay_marker )
// {
//     if ( ! M_formation )
//     {
//         return;
//     }

//     if ( M_formation->updateMarker( num, marker, setplay_marker ) )
//     {
//         M_conf_changed = true;
//     }
// }

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::moveBallTo( const double x,
                      const double y )
{
    Vector2D pos = round_coordinates( x, y );

    M_current_state.ball_ = pos;

    if ( Options::instance().dataAutoSelect() )
    {
        if ( pos.absY() < 1.0 )
        {
            M_current_state.ball_.y = 0.0;
        }

        if ( M_formation_data )
        {
            const int idx = M_formation_data->nearestDataIndex( pos, 1.0 );
            const FormationData::Data * data = M_formation_data->data( idx );
            if ( data )
            {
                M_current_index = idx;
                M_current_state.ball_ = data->ball_;
            }
        }
    }

    updatePlayerPosition();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::movePlayerTo( const int num,
                        const double x,
                        const double y )
{
    if ( num < 1 || 11 < num )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " movePlayerTo() Illegal num " << num
                  << std::endl;
        return;
    }

    try
    {
        Vector2D pos = round_coordinates( x, y );

        M_current_state.players_.at( num - 1 ) = pos;

        if ( Options::instance().pairMode()
             && M_current_state.ball_.absY() < 0.5
             && M_formation )
        {
            const int pair = M_formation->pairedNumber( num );
            if ( 1 <= pair && pair <= 11 )
            {
                M_current_state.players_.at( pair - 1 ).assign( x, -y );
            }
            else
            {
                for ( int n = 1; n <= 11; ++n )
                {
                    if ( n == num ) continue;
                    if ( M_formation->pairedNumber( n ) == num )
                    {
                        M_current_state.players_.at( n - 1 ).assign( x, -y );
                    }
                }
            }
        }
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " movePlayerTo() exception caught! " << e.what()
                  << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
// void
// EditData::setConstraintTerminal( const double x,
//                                  const double y )
// {
//     Vector2D pos = round_coordinates( x, y );

//     M_constraint_terminal_index = -1;
//     M_constraint_terminal = pos;

//     // automatically select terminal vertex
//     if ( M_formation_data )
//     {
//         const int idx = M_formation_data->nearestDataIndex( pos, 1.0 );
//         const FormationData::Data * data = M_formation_data->data( idx );
//         if ( M_constraint_origin_index != idx
//              && data )
//         {
//             M_constraint_terminal_index = idx;
//             M_constraint_terminal = data->ball_;
//         }
//     }
// }


/*-------------------------------------------------------------------*/
/*!

 */
// void
// EditData::setConstraintIndex( const int origin_idx,
//                               const int terminal_idx )
// {
//     if ( ! M_formation_data )
//     {
//         return;
//     }

//     if ( origin_idx < 0 )
//     {
//         M_select_type = NO_SELECT;
//         M_select_index = 0;
//         M_constraint_origin_index = -1;
//         M_constraint_terminal_index = -1;
//         M_constraint_terminal = Vector2D::INVALIDATED;
//         return;
//     }

//     if ( static_cast< int >( M_formation_data->dataCont().size() ) < origin_idx + 1 )
//     {
//         std::cerr << "(EditData::setConstraintIndex) origin index over range."
//                   << " origin=" << origin_idx
//                   << " terminal=" << terminal_idx
//                   << std::endl;
//         return;
//     }

//     if ( terminal_idx < 0 )
//     {
//         M_select_type = SELECT_SAMPLE;
//         M_select_index = origin_idx;
//         M_constraint_origin_index = origin_idx;
//         M_constraint_terminal_index = -1;
//         return;
//     }

//     if ( static_cast< int >( M_formation_data->dataCont().size() ) < terminal_idx + 1 )
//     {
//         std::cerr << "(EditData::setConstraintIndex) terminal index over range."
//                   << " origin=" << origin_idx
//                   << " terminal=" << terminal_idx
//                   << std::endl;
//         return;
//     }

//     if ( origin_idx == terminal_idx )
//     {
//         M_select_type = SELECT_SAMPLE;
//         M_select_index = origin_idx;
//         M_constraint_origin_index = origin_idx;
//         return;
//     }

//     M_select_type = SELECT_SAMPLE;
//     M_select_index = origin_idx;
//     M_constraint_origin_index = origin_idx;
//     M_constraint_terminal_index = terminal_idx;

//     FormationData::DataCont::const_iterator it = M_formation_data->dataCont().begin();
//     std::advance( it, terminal_idx );

//     M_constraint_terminal = it->ball_;
// }

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::moveSelectObjectTo( const double x,
                              const double y )
{
    switch ( selectType() ) {
    case NO_SELECT:
        return false;
    case SELECT_BALL:
        moveBallTo( x, y );
        return true;
    case SELECT_PLAYER:
        movePlayerTo( selectIndex() + 1, x, y );
        return true;
    case SELECT_SAMPLE:
        // setConstraintTerminal( x, y );
        return true;
    default:
        break;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::selectObject( const double x,
                        const double y )
{
    const Vector2D pos( x, y );
    const double dist2_thr = 1.5 * 1.5;

    double mindist2 = 200.0 * 200.0;

    if ( Options::instance().constraintEditMode() )
    {
        //
        // data
        //
        if ( M_formation_data )
        {
            size_t index = 0;
            for ( const FormationData::Data & d : M_formation_data->dataCont() )
            {
                double d2 = d.ball_.dist2( pos );
                if ( d2 < dist2_thr
                     && d2 < mindist2 )
                {
                    M_select_type = SELECT_SAMPLE;
                    M_select_index = index; //std::distance( M_triangulation.indexedVertices().begin(), v );
                    M_constraint_origin_index = index;
                    mindist2 = d2;
                }
                ++index;
            }
        }
    }
    else
    {
        //
        // ball
        //
        {
            double d2 = M_current_state.ball_.dist2( pos );
            if ( d2 < dist2_thr )
            {
                //std::cerr << "selection update ball" << std::endl;
                M_select_type = SELECT_BALL;
                mindist2 = d2;
            }
        }

        //
        // players
        //
        size_t index = 0;
        for ( std::vector< Vector2D >::iterator it = M_current_state.players_.begin();
              it != M_current_state.players_.end();
              ++it, ++index )
        {
            double d2 = it->dist2( pos );
            if ( d2 < dist2_thr
                 && d2 < mindist2 )
            {
                M_select_type = SELECT_PLAYER;
                M_select_index = index; //std::distance( M_current_state.players_.begin(), it );
                mindist2 = d2;
            }
        }
    }

    if ( M_select_type == SELECT_BALL )
    {
        M_current_state.ball_ = pos;
    }
    else if ( M_select_type == SELECT_PLAYER )
    {
        M_current_state.players_[M_select_index] = pos;
    }
    else if ( M_select_type == SELECT_SAMPLE )
    {
        M_constraint_terminal_index = -1;
        M_constraint_terminal = Vector2D::INVALIDATED;
    }

    return ( M_select_type != NO_SELECT );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::releaseObject()
{
    if ( M_select_type != NO_SELECT )
    {
        M_select_type = NO_SELECT;
        M_constraint_origin_index = -1;
        M_constraint_terminal_index = -1;
        M_constraint_terminal = Vector2D::INVALIDATED;
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
EditData::addData()
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    // add data
    std::string err = M_formation_data->addData( M_current_state );
    if ( ! err.empty() )
    {

    }

    // add paired data
    if ( Options::instance().pairMode()
         && M_current_state.ball_.absY() >= 0.5 )
    {
        FormationData::Data reversed = M_current_state;
        reversed.ball_.y *= -1.0;
        reverseY( &reversed.players_ );
        err = M_formation_data->addData( reversed );
        if ( ! err.empty() )
        {

        }
    }

    M_current_state = M_formation_data->dataCont().back();
    M_current_index = M_formation_data->dataCont().size() - 1;

    train();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
EditData::insertData( const int idx )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    if ( idx < 0 )
    {
        return std::string( "Invalid index" );
    }

    std::string err = M_formation_data->insertData( static_cast< size_t >( idx ), M_current_state );
    if ( ! err.empty() )
    {
        return err;
    }

    if ( Options::instance().pairMode() )
    {
        FormationData::Data reversed_data = M_current_state;
        reversed_data.ball_.y *= -1.0;
        reverseY( &reversed_data.players_ );

        err = M_formation_data->insertData( static_cast< size_t >( idx + 1 ), reversed_data );
        if ( ! err.empty() )
        {
            return err;
        }
    }


    M_current_index = idx;

    train();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
EditData::replaceData( const int idx )
{
    return replaceDataImpl( idx, M_current_state );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
EditData::replaceDataImpl( const int idx,
                           const FormationData::Data & data )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }


    const FormationData::Data * original_data = M_formation_data->data( static_cast< size_t >( idx ) );
    if ( ! original_data )
    {
        return std::string( "Invalid index" );
    }
    const Vector2D original_ball = original_data->ball_;

    // replace data
    {
        std::string err = M_formation_data->replaceData( static_cast< size_t >( idx ), data );

        if ( ! err.empty() )
        {
            return err;
        }
    }

    // replace the paired data
    if ( Options::instance().pairMode()
         && data.ball_.absY() >= 0.5 )
    {
        size_t reversed_idx = size_t( -1 );

        for ( size_t i = 0; i < M_formation_data->dataCont().size(); ++i )
        {
            const FormationData::Data * r = M_formation_data->data( i );
            if ( r
                 && std::fabs( r->ball_.x - original_ball.x ) < 1.0e-5
                 && std::fabs( r->ball_.y + original_ball.y ) < 1.0e-5 )
            {
                reversed_idx = i;
                break;
            }
        }

        FormationData::Data reversed_data = data;
        reversed_data.ball_.y *= -1.0;
        reverseY( &reversed_data.players_ );

        if ( reversed_idx != size_t( -1 ) )
        {
            std::string err = M_formation_data->replaceData( reversed_idx, reversed_data );
            if ( ! err.empty() )
            {
                return err;
            }
        }
        else
        {
            std::string err = M_formation_data->addData( reversed_data );
            if ( ! err.empty() )
            {
                std::cerr << "(EditData::replaceDataImpl) ERROR?" << std::endl;
            }
        }
    }

    train();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
EditData::replaceBall( const int idx,
                       const double x,
                       const double y )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    const FormationData::Data * d = M_formation_data->data( static_cast< size_t >( idx ) );

    if ( ! d )
    {
        return std::string( "Invalid index" );
    }

    FormationData::Data tmp = *d;
    tmp.ball_.assign( x, y );

    return replaceDataImpl( idx, tmp );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
EditData::replacePlayer( const int idx,
                         const int num,
                         const double x,
                         const double y )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    const FormationData::Data * d = M_formation_data->data( static_cast< size_t >( idx ) );

    if ( ! d )
    {
        return std::string( "Invalid index" );
    }

    FormationData::Data tmp = *d;
    try
    {
        tmp.players_.at( num - 1 ).assign( x, y );
    }
    catch ( std::exception & e )
    {
        std::cerr << e.what()
                  << ": EditData::replacePlayer() illegal player number. "
                  << num << std::endl;
        return std::string( "Illegal player number" );
    }

    return replaceDataImpl( idx, tmp );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
EditData::deleteData( const int idx )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    std::string err = M_formation_data->removeData( static_cast< size_t >( idx ) );

    if ( ! err.empty() )
    {
        return err;
    }

    M_current_index = -1;

    train();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
EditData::changeDataIndex( const int old_idx,
                           const int new_idx )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    size_t old_index = static_cast< size_t >( old_idx );
    size_t new_index = static_cast< size_t >( new_idx );

    if ( new_idx > old_idx )
    {
        new_index += 1;
    }

    std::string err = M_formation_data->changeDataIndex( old_index, new_index );

    if ( ! err.empty() )
    {
        return err;
    }

    std::cerr << "move data from " << old_idx << " to " << new_idx << std::endl;

    M_current_index = new_idx;

    train();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
// std::string
// EditData::addConstraint( const int origin_idx,
//                          const int terminal_idx )
// {
//     if ( ! M_formation
//          || ! M_formation_data )
//     {
//         return std::string( "No formation" );
//     }

//     size_t origin = static_cast< size_t >( std::min( origin_idx, terminal_idx ) );
//     size_t terminal = static_cast< size_t >( std::max( origin_idx, terminal_idx ) );

//     std::string err = M_formation_data->addConstraint( origin, terminal );

//     if ( ! err.empty() )
//     {
//         return err;
//     }

//     std::cerr << "add constraint (" << origin << ',' << terminal << ')' << std::endl;

//     train();

//     return std::string();
// }

/*-------------------------------------------------------------------*/
/*!

 */
// std::string
// EditData::replaceConstraint( const int idx,
//                              const int origin_idx,
//                              const int terminal_idx )
// {
//     if ( ! M_formation
//          || ! M_formation_data )
//     {
//         return std::string( "No formation" );
//     }

//     std::string err = M_formation_data->replaceConstraint( static_cast< size_t >( idx ),
//                                                            static_cast< size_t >( origin_idx ),
//                                                            static_cast< size_t >( terminal_idx ) );

//     if ( ! err.empty() )
//     {
//         return err;
//     }

//     std::cerr << "replace constraint " << idx
//               << " to (" << origin_idx << ',' << terminal_idx << ')'
//               << std::endl;

//     train();

//     return std::string();
// }

/*-------------------------------------------------------------------*/
/*!

 */
// std::string
// EditData::deleteConstraint( const int origin_idx,
//                             const int terminal_idx )
// {
//     if ( ! M_formation
//          || ! M_formation_data )
//     {
//         return std::string( "No formation" );
//     }

//     std::string err= M_formation_data->removeConstraint( static_cast< size_t >( origin_idx ),
//                                                          static_cast< size_t >( terminal_idx ) );

//     if ( ! err.empty() )
//     {
//         return err;
//     }

//     std::cerr << "delete constraint (" << origin_idx << ',' << terminal_idx << ')'
//               << std::endl;

//     train();

//     return std::string();
// }

/*-------------------------------------------------------------------*/
/*!

 */
bool
EditData::setCurrentIndex( const int idx )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return false;
    }

    if ( idx == -1 )
    {
        M_current_index = idx;
        return true;
    }

    const FormationData::Data * d = M_formation_data->data( idx );
    if ( ! d )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << "Index range over. " << idx
                  << std::endl;
        return false;
    }

    M_current_index = idx;

    if ( Options::instance().playerAutoMove() )
    {
        M_current_state = *d;
    }
    else
    {
        M_current_state.ball_ = d->ball_;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::reverseY( std::vector< Vector2D > * players )
{
    if ( ! M_formation )
    {
        std::cerr << "(EditData::reverseY) ***ERROR*** No formation." << std::endl;
        return;
    }

    std::vector< Vector2D > old_players = *players;

    int num = 0;
    for ( Vector2D & p : *players )
    {
        ++num;

        const int pair = M_formation->pairedNumber( num );
        if ( pair == 0 )
        {
            p.y *= -1.0;
        }
        else if ( 1 <= pair && pair <= 11 )
        {
            p.x = old_players[pair - 1].x;
            p.y = old_players[pair - 1].y * -1.0;
        }
        else
        {
            for ( int n = 1; n <= 11; ++n )
            {
                if ( M_formation->pairedNumber( n ) == num )
                {
                    p.x = old_players[n - 1].x;
                    p.y = old_players[n - 1].y * -1.0;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::reverseY()
{
    if ( ! M_formation )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " ***ERROR*** No formation!!"
                  << std::endl;
        return;
    }

    M_current_state.ball_.y *= -1.0;

    reverseY( &M_current_state.players_ );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
EditData::train()
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return;
    }

    M_formation->train( *M_formation_data );
    M_conf_changed = true;
    updatePlayerPosition();
    updateTriangulation();
}
