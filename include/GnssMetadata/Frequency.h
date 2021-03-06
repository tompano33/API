/**
 * File: Frequency.h
 * Author: M.B. Mathews
 *  
 * Copyright(c) 2014 Institute of Navigation
 * http://www.ion.org
 *  
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef FREQUENCY_H_H
#define FREQUENCY_H_H

#include "BaseTypes.h"
namespace GnssMetadata
{
	/**
	 * Class represents an frequency representable in multiple units.   Default unit format is Hz.
	 */
	class Frequency: public Object
	{
	public:
		enum FrequencyFormat
		{
			Hz,
			MHz,
			GHz,
			Ratio
		};

		union ValueType
		{
			double doubleVal;
			struct { long numerator; long denominator;} ratioVal;
		};

	public:
		Frequency( const double value = 0.0, FrequencyFormat fmt = Hz) : _format(fmt)
		{
			_value.doubleVal = value;
		}

		Frequency( long numerator, long denominator) : _format( Ratio)
		{
			_value.ratioVal.numerator = numerator;
			_value.ratioVal.denominator = denominator; 
		}
		
		Frequency( const Frequency& rhs) : _value(rhs._value), _format(rhs._format)
		{}


		FrequencyFormat Format( ) const
		{
			return _format;
		}

		ValueType Value( ) const
		{
			return _value;
		}
		void Value( const double value, FrequencyFormat fmt = Hz )
		{
			_value.doubleVal = value;
			_format = fmt;
		}

		void Value( long numerator, long denominator)
		{
			_value.ratioVal.numerator = numerator;
			_value.ratioVal.denominator = denominator; 
			_format = Ratio;
		}

		/**
		 * Returns a string representation of the object.
		 */
		virtual String toString( const String& sFormat =  DefaultFormat );
		
	private:
		/**
		 * The frequency value.
		 */
		ValueType _value;

		FrequencyFormat _format;
		
		
	};
	
}


#endif
