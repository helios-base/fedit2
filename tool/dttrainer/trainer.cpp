// -*-c++-*-

/*!
  \file trainer.cpp
  \brief trainer for FormationDT Source File.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "trainer.h"

#include <iostream>
#include <fstream>
#include <cstdio>

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
Trainer::Trainer()
    : M_alpha( 0.1 )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Trainer::readFormation( const std::string & filepath )
{
    std::ifstream fin( filepath.c_str() );

    if ( ! fin )
    {
        return false;
    }

    Formation::Ptr ptr = Formation::create( fin );

    if ( ! ptr )
    {
        return false;
    }

    fin.seekg( 0 );
    if ( ! ptr->read( fin ) )
    {
        return false;
    }

    fin.close();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Trainer::readTrainingData( const std::string & filepath )
{
    // format:
    //
    //  (b <X> <Y>) (<UNUM> <X> <Y>)
    //

    std::ifstream fin( filepath.c_str() );

    if ( ! fin )
    {
        return false;
    }

    std::string line;
    int n_line = 0;
    while ( std::getline( fin, line ) )
    {
        ++n_line;

        int unum;
        double bx, by, px, py;
        if ( std::sscanf( line.c_str(),
                          " ( b %lf %lf ) ( %d %lf %lf )",
                          &bx, &by, &unum, &px, &py ) != 5 )
        {
            std::cerr << "line " << n_line
                      << " illegal data [" << line << "]" << std::endl;
            return false;
        }

        if ( unum < 0 || 11 < unum )
        {
            std::cerr << "line " << n_line
                      << " illegal unum [" << line << "]" << std::endl;
            return false;
        }

        M_data.push_back( Data( bx, by, unum, px, py ) );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Trainer::train()
{

    return false;
}
