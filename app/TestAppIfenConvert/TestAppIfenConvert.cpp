/**
 * File: TestAppIfenConvert.cpp
 * Author: T. Pany
 *  
 * Copyright(c) 2014 Institute of Navigation
 * http://www.ion.org
 *
 * The program demonstrates the converion of IFEN internal meta data 
 * structures to/from the ION standard.
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

// standard C++ include files
#include <math.h>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

// include files for ION GNSS metadata standard
#include<GnssMetadata/Metadata.h>
#include<GnssMetadata/Xml/XmlProcessor.h>
using namespace GnssMetadata;



// some IFEN specific typedefs
typedef int nsr_int32;
typedef long long nsr_int64;

//! Modified class from IFEN rx to create and manage high precision time and to convert it to/from different time systems 
class PreciseTime
{
	public:
		//! Number of seconds (integer part of the precise time).
		nsr_int64	nTime;
		//! Sub-second (fractional part of the precise time). 0.0<= dValue <1.0. In fact, the real time value = nTime.0 + dTime.
		double	dTime;			

}; // end of class PreciseTime

//! Modified data structure of IFEN software receiver to represent metadata
class DSamplePacketInfo
{
	public:
		// max. number of processed RF bands
		const static int nMaxSimultaneousStreams=8;

		//! type of sample input
		enum SampleInputSource
		{
			File,	// read data from a file, whose configuration is stored previously as an XML format. Now a ClassicDataFile format is used.
			NavPortUSB,	// NavPort frontend conntected via USB 2.0
			NavPort2USB,// NavPort2 frontend conntected via USB 2.0
			NavPort4USB,// NavPort4 frontend conntected via USB 2.0
			NavPort5USB,// SX3 conntected via USB 3.0
			FausstUSB,  // FAUSST frontend conntected via USB
			NI,			// NI card
			Audio,		// Uses sound card input
			NI2,		// Dual NI
			SourceUnknown		// No real source, used for initialization or test purposes
		} nSource;
		//! Number of streams.
		nsr_int32	nStreams;	
		//! Time of first sample of stream (GPS time scale).
		PreciseTime	ptCoarseStartTime;	
		//! Frontend Number.
		nsr_int32	vnFrontendNo[nMaxSimultaneousStreams];
		//! Antenna ID.
		nsr_int32	vnAntennaId[nMaxSimultaneousStreams];
		//! RF Band (Channel) Number of the virtual bands per Frontend. For each virtual band i in vnBandNo[i] the corresponding RF band number is given by vnRfBandNo[i]
		nsr_int32	vnRfBandNo[nMaxSimultaneousStreams];
		//! Virtual Band (Channel) Number per Frontend, see vector vnRfBandNo for RF band number for virtual stream 
		nsr_int32	vnBandNo[nMaxSimultaneousStreams];
		//! fz, numerator to calculate the sample rate.
		nsr_int32 vnSampleRateZ[nMaxSimultaneousStreams];
		//! fn, denominator to calculate the sample rate.
		nsr_int32 vnSampleRateN[nMaxSimultaneousStreams];
		//! IF frequency for each stream.
		double vdIntermediateFrequency[nMaxSimultaneousStreams];
		//! Carrier frequency for each virtual stream.
		double vdCarrierFrequency[nMaxSimultaneousStreams];
		//! 0 = 2-bit samples, 1 = 4-bit samples, 2 = 8-bit samples, to be continued
		int vnFileFormat[nMaxSimultaneousStreams];
		//! file names of binary sample data
		string vstrFileNames[nMaxSimultaneousStreams];
};



/**
 * Loads a metadata file.   
 */
Metadata ReadXmlFile(const char* pszFilename)
{
    printf("\nReading GNSS Metadata from XML file: %s\n", pszFilename);

	Metadata md;
    XmlProcessor xproc;

    try
    {
        if( xproc.Load( pszFilename, false, md) )
        {
            printf("Xml Processed successfully.\n");


        }
    }
    catch( ApiException& e)
    {
        printf("An error occurred while saving the xml file: %s\n", e.what() );
    }
    catch( std::exception& ge)
    {
        printf("Unknown Exception Occured: %s\n", ge.what() );
    }

	return md;
}



/**
 * Writes a metadata file.   
 */
void WriteXmlFile( const Metadata md, const char* pszFilename)
{
    printf("\nWriting GNSS Metadata to xml file: %s\n", pszFilename);

	XmlProcessor proc;

	try
	{
		proc.Save( pszFilename,  md);
	}
	catch( ApiException& e)
	{
		printf("An error occurred while saving the xml file: %s\n", e.what() );
	}
}



/**
 * Converts metadata from IFEN structure to ION data structure
 */
Metadata convertFromIfen( const DSamplePacketInfo si )
{
	Metadata md;
	string equip;
	const char* fe_type[] = { "file", "", "", "NavPort-4", "SX3", "", "" };
	int i;
	Lump lump;
	Band band;
	Stream stream;
	Chunk chunk;
	Block block;
	Lane lane;
	File file;
	FileSet fileset;

	// extract TOA
	Date dt0( fmod(si.ptCoarseStartTime.nTime+si.ptCoarseStartTime.dTime, 86400.*7 ), int( ( si.ptCoarseStartTime.nTime + si.ptCoarseStartTime.dTime)/(86400*7) ) );
	size_t offset = 0;

	// Define the Session.
	Session sess("");
	sess.Scenario("");
	sess.Campaign("");
	sess.Contact("");
	sess.Toa( dt0 );
	sess.AddComment("Two spatially spitted files (multi-file recording)");

	//Define the System, Sources, and cluster.
	System sys("MySystem");
	sys.BaseFrequency( Frequency( 40, Frequency::MHz));
	sys.Equipment( string( "IFEN frontend type: " ) + string( fe_type[si.nSource] ) );

	// first prepare file set
	fileset = FileSet( "MyFileSet" );

	for( i = 0; i < si.nStreams; i++ )
	{
		std::ostringstream out;
		out << "RfBand" << si.vnRfBandNo[i] << "_Band" << si.vnBandNo[i]; 
		band=Band( out.str( ) );
		band.CenterFrequency(Frequency( si.vdCarrierFrequency[i]/1e6, Frequency::MHz));
		band.TranslatedFrequency(Frequency( si.vdIntermediateFrequency[i]/1e6, Frequency::MHz));
		
		stream=Stream("MyStream");
		stream.RateFactor(1);
		stream.Quantization(2);
		stream.Packedbits(8);
		stream.Encoding("INT8");
		stream.Format(Stream::IF);
		stream.Bands().push_back( band );
		
		lump=Lump( "" );
		lump.Streams().push_back( stream );

		chunk=Chunk("");
		chunk.SizeWord(1);
		chunk.CountWords(1);
		chunk.Lumps().push_back( lump );
		chunk.Endian( Chunk::WordEndian::Big );

		block=Block("");
		block.Chunks().push_back( chunk );
	
		lane=Lane("MyLane");
		lane.Sessions().push_back( sess );
		lane.Systems( ).push_back( sys );
		lane.Blocks().push_back( block );


		file=File(si.vstrFileNames[i] );
		file.Url( si.vstrFileNames[i] );
		file.TimeStamp( dt0 );
		file.Lane( lane, true );

		fileset.FileUrls( ).push_back( file.toString( ) );

		md.Lanes( ).push_back( lane );
		md.Files( ).push_back( file );
	}

	md.FileSets( ).push_back( fileset );
	md.AddComment( "IFEN GNSS metadata converter test" );

	return md;
}



/**
 * Converts metadat from ION data structure to IFEN structure
 */
DSamplePacketInfo convertToIfen( const Metadata md )
{
	DSamplePacketInfo mySampInfo;

	return mySampInfo;
}



/**
 * main routine (see beginning of file for description)
 */
int main(int argc, char** argv)
{
	DSamplePacketInfo mySamples1, mySamples2; // IFEN struct
	Metadata md1, md2, md3;
	int i;

	printf("GNSS Metadata XML <-> IFEN conversion test\n");
	printf("Test successful, if output file 'fe0.setx' matches 'fe0_1.setx" );
    printf("\n");

	// setup IFEN internal struct
	mySamples1.nStreams = 2;
	mySamples1.nSource = DSamplePacketInfo::NavPort5USB;
	mySamples1.ptCoarseStartTime.nTime =1915*86400*7; // GPS week 1915, DOW: 3, TOD: 8:00:00
	mySamples1.ptCoarseStartTime.dTime =86400*3+8*3600;
	// stream 1
	mySamples1.vdCarrierFrequency[0] = 1.57542e9;
	mySamples1.vdIntermediateFrequency[0] = 19.942e6;
	mySamples1.vnSampleRateN[0] = 1;
	mySamples1.vnSampleRateZ[0] = 40000000;
	mySamples1.vnAntennaId[0] = 0;
	mySamples1.vnBandNo[0] = 0;
	mySamples1.vnRfBandNo[0] = 0;
	mySamples1.vnFrontendNo[0] = 0;
	mySamples1.vnFileFormat[0] = 0;
	mySamples1.vstrFileNames[0] = "fe0_band0.stream";
	// stream 2
	mySamples1.vdCarrierFrequency[1] = 1.602e9;
	mySamples1.vdIntermediateFrequency[1] = 19.912e6;
	mySamples1.vnSampleRateN[1] = 1;
	mySamples1.vnSampleRateZ[1] = 40000000;
	mySamples1.vnAntennaId[1] = 0;
	mySamples1.vnBandNo[1] = 1;
	mySamples1.vnRfBandNo[1] = 0;
	mySamples1.vnFrontendNo[1] = 0;
	mySamples1.vnFileFormat[1] = 0;
	mySamples1.vstrFileNames[1] = "fe0_band1.stream";




	md1 = convertFromIfen( mySamples1 );

	WriteXmlFile( md1, "fe0.setx" );
    md2 = ReadXmlFile( "fe0.setx" );
	WriteXmlFile( md2, "fe0_1.setx" );
	//mySamples2 = convertToIfen( md2 );
	//md3 = convertFromIfen( mySamples2 );
    //WriteXmlFile( md3, file2 );

	return 0;
}

