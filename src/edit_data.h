// -*-c++-*-

/*!
  \file edit_data.h
  \brief formation editor data class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
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

#ifndef FEDIT2_EDIT_DATA_H
#define FEDIT2_EDIT_DATA_H

#include <QString>

#include <rcsc/formation/formation_data.h>
#include <rcsc/formation/formation.h>
#include <rcsc/geom/triangulation.h>
#include <rcsc/geom/vector_2d.h>


class EditData {
public:

    static const double MAX_X;
    static const double MAX_Y;

    enum SelectType {
        SELECT_BALL,
        SELECT_PLAYER,
        SELECT_SAMPLE,
        NO_SELECT,
    };

private:

    QString M_filepath;
    bool M_conf_changed;
    QString M_saved_datetime;

    rcsc::FormationData::Data M_current_state; //!< current state on the edit canvas.

    rcsc::Formation::Ptr M_formation;
    rcsc::FormationData::Ptr M_formation_data; //!< training data
    rcsc::Triangulation M_triangulation;

    rcsc::Formation::Ptr M_background_formation;
    rcsc::Triangulation M_background_triangulation;

    int M_current_index;

    SelectType M_select_type;
    int M_select_index;

    int M_constraint_origin_index;
    int M_constraint_terminal_index;
    rcsc::Vector2D M_constraint_terminal;

    // not used
    EditData( const EditData & );
    EditData & operator=( const EditData & );
public:

    EditData();
    ~EditData();

    void createFormation( const QString & type_name );
private:

    void init();
    void backup( const QString & filepath );

public:

    const
    QString & filePath() const
    {
        return M_filepath;
    }

    bool isConfChanged() const
    {
        return M_conf_changed;
    }

    const rcsc::FormationData::Data & currentState() const
    {
        return M_current_state;
    }

    rcsc::Formation::ConstPtr formation() const
    {
        return M_formation;
    }
    rcsc::FormationData::Ptr formationData() const
    {
        return M_formation_data;
    }
    const
    rcsc::Triangulation & triangulation() const
    {
        return M_triangulation;
    }

    rcsc::Formation::ConstPtr backgroundFormation() const
    {
        return M_background_formation;
    }
    const
    rcsc::Triangulation & backgroundTriangulation() const
    {
        return M_background_triangulation;
    }

    int currentIndex() const
    {
        return M_current_index;
    }

    SelectType selectType() const
    {
        return M_select_type;
    }
    size_t selectIndex() const
    {
        return M_select_index;
    }

    size_t constraintOriginIndex() const
    {
        return M_constraint_origin_index;
    }

    size_t constraintTerminalIndex() const
    {
        return M_constraint_terminal_index;
    }

    const
    rcsc::Vector2D & constraintTerminal() const
    {
        return M_constraint_terminal;
    }

    bool openConf( const QString & filepath );
    bool saveConf();
    bool saveConfAs( const QString & filepath );

    bool openData( const QString & filepath );
    bool saveDataAs( const QString & filepath );

    bool openBackgroundConf( const QString & filepath );

private:
    void updatePlayerPosition();
    void updateTriangulation();

    void reverseY( std::vector< rcsc::Vector2D > * players );

    std::string replaceDataImpl( const int idx,
                                 const rcsc::FormationData::Data & data );
public:

    void updateRoleData( const int num,
                         const int paird_unum,
                         const std::string & role_name );
    void updateRoleType( const int num,
                         const int type_index,
                         const int side_index );
    // void updateMarkerData( const int unum,
    //                        const bool marker,
    //                        const bool setplay_marker );

    void moveBallTo( const double x,
                     const double y );
    void movePlayerTo( const int num,
                       const double x,
                       const double y );
    // void setConstraintTerminal( const double x,
    //                             const double y );
    // void setConstraintIndex( const int origin_idx,
    //                          const int terminal_idx );
    bool moveSelectObjectTo( const double x,
                             const double y );

    bool selectObject( const double x,
                       const double y );
    bool releaseObject();

    std::string addData();
    std::string insertData( const int idx );
    std::string replaceData( const int idx );
    std::string replaceBall( const int idx,
                             const double x,
                             const double y );
    std::string replacePlayer( const int idx,
                               const int unum,
                               const double x,
                               const double y );
    std::string deleteData( const int idx );
    std::string changeDataIndex( const int old_idx,
                                 const int new_idx );

    // std::string addConstraint( const int origin_idx,
    //                            const int terminal_idx );
    // std::string replaceConstraint( const int idx,
    //                                const int origin_idx,
    //                                const int terminal_idx );
    // std::string deleteConstraint( const int origin_idx,
    //                               const int terminal_idx );


    bool setCurrentIndex( const int idx );
    void reverseY();
    void train();
};

#endif
