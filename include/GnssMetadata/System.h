/**
 * File: System.h
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

#ifndef SYSTEM_H_H
#define SYSTEM_H_H

#include "AttributedObject.h"
#include "BaseTypes.h"
#include "Source.h"
#include "Cluster.h"
#include "Frequency.h"
#include <list>

namespace GnssMetadata
{
	/**
	 * Class defines system metadata attributes.
	 */
	class System: public AttributedObject
	{
	public:
		enum SystemType
		{
			Undefined=0,
			Processor,
			Receiver,
			Simulator
		};
	public:
		System( const String& id = "", bool bIsReference = false, SystemType type=Undefined) 
			: AttributedObject( id, bIsReference), _type( type) {}
		System(const System& rhs) 
			: AttributedObject( rhs), _freqBase( rhs._freqBase), 
			_equipment( rhs._equipment), _type( rhs._type), 
			_sourcelist(rhs._sourcelist),
			_clusterlist(rhs._clusterlist)
		{}

		const System& operator=( const System& rhs)
		{
			if( &rhs == this)
				return *this;
			AttributedObject::operator =(rhs);
			_freqBase = rhs._freqBase;
			_equipment = rhs._equipment;
			_type = rhs._type;
			_sourcelist = rhs._sourcelist;
			_clusterlist = rhs._clusterlist;
			return *this;
		}

		const Frequency& BaseFrequency( ) const
		{
			return _freqBase;
		}
		Frequency& BaseFrequency( ) 
		{
			return _freqBase;
		}
		void BaseFrequency( const Frequency& centerFrequency )
		{
			_freqBase = centerFrequency;
		}

		/**
		 * Gets the system equipment information.
		 */
		String Equipment( ) const
		{
			return _equipment;
		}
		/**
		 * Sets the system equipment information.
		 */
		void Equipment( const String& equipment )
		{
			_equipment = equipment;
		}
		/**
		 * Sets the System type.
		 */
		void Type( const SystemType type )
		{
			_type = type;
		}
		/**
		 * Gets the System type.
		 */
		SystemType Type( ) const
		{
			return _type;
		}

		const GnssMetadata::SourceList& Sources( ) const
		{
			return _sourcelist;
		}
		GnssMetadata::SourceList& Sources( )
		{
			return _sourcelist;
		}
		void AddSource( const GnssMetadata::Source& source)
		{
			_sourcelist.push_back( source);
		}

		const GnssMetadata::ClusterList& Clusters( ) const
		{
			return _clusterlist;
		}
		GnssMetadata::ClusterList& Clusters( )
		{
			return _clusterlist;
		}
		void AddCluster( const GnssMetadata::Cluster& cluster)
		{
			_clusterlist.push_back( cluster);
		}



		/**
		 *  Returns a string representation of the object.
		 *  
		 */
		virtual String toString( const String & sFormat = DefaultFormat );
		
	private:
		GnssMetadata::Frequency _freqBase;


		/**
		 * Specifies the equipment information for the system.
		 */
		String _equipment;

		/**
		 * Specifies the general type of the system responsible for producing the data.
		 */
		SystemType _type;

		GnssMetadata::SourceList _sourcelist;
		GnssMetadata::ClusterList _clusterlist;
	};

	typedef std::list<System> SystemList;
	
}


#endif
