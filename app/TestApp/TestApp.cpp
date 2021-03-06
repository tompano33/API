/**
 * File: TestApp.cpp
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
#include<GnssMetadata/Metadata.h>
#include<GnssMetadata/Xml/XmlProcessor.h>

using namespace GnssMetadata;

void ReadXmlFile(const char* pszFilename);
void WriteXmlFile(const char* pszFilename);


int main(int argc, char** argv)
{
    //Get the filename if specified.
    const char* pszFilename = "141230-gps-4msps.xml";
    if( argc > 1) pszFilename = argv[1];

    printf("GNSS Metadata XML file translation test\n");
    printf("\n");
    printf("Application demonstrates writing and reading an XML file\n");
    printf("Program creates an metadata file using the API and then parses it.\n");
    printf("\n");
    printf("Command line\n");
    printf("GnssMetadataTestApp [xmlfile (default: '14230-gps-4msps.xml')]\n");

    WriteXmlFile(pszFilename);
    ReadXmlFile(pszFilename);
	return 0;
}

/**
 * Loads and saves a metadata file.   
 */
 void ReadXmlFile(const char* pszFilename)
{
    printf("\nReading GNSS Metadata to xml file: %s\n", pszFilename);

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


}

void WriteXmlFile(const char* pszFilename)
{
    printf("\nWriting GNSS Metadata to xml file: %s\n", pszFilename);

    ////////////////////////////////
	//Define information about the datafile.

	//UTC:    30-Dec-2014 22:38:54
	//GPS:    1825/254334.906
	Date dt0( 254334.906, 1825);
	size_t offset = 588;
    String sfile = "141230-gps-4msps.bds";
    String sfilemd = pszFilename;

	////////////////////////////////
	//Define the Session.
	Session sess("0");
	sess.Scenario("Example 2");
	sess.Campaign("GNSS Metadata API Testing");
	sess.Contact("J. Doe");
	sess.Position( Position(47.76471300, -122.15612900, 25.610));
	sess.AddComment("This is an example metadata specification with two interleaved streams.");

	////////////////////////////////
	//Define the System, Sources, and cluster.
	System sys("A2300-1");
	sys.BaseFrequency( Frequency( 4, Frequency::MHz));
	sys.Equipment("ASR-2300");
	sys.AddComment( "ASR-2300 configured with standard firmware and FPGA id=1, version=1.18.");

	Cluster clstr("Antenna");

	Source src1( Source::Patch, Source::RHCP, "L1 C/A");
	src1.IdCluster("Antenna");

	Source src2( Source::Patch, Source::RHCP, "L2 C");
	src2.IdCluster("Antenna");

	sys.AddSource( src1);
	sys.AddSource( src2);
	sys.AddCluster(clstr);

	////////////////////////////////
	//Define Band 1 and L1 C/A Stream.
	Band ch1("L1External");
	ch1.CenterFrequency(Frequency( 1575.42, Frequency::MHz));
	ch1.TranslatedFrequency(Frequency( 38400, Frequency::Hz));

	Stream sm1("L1ca");
	sm1.RateFactor(1);
	sm1.Quantization(8);
	sm1.Packedbits(16);
	sm1.Encoding("INT8");
	sm1.Format(Stream::IQ);
	sm1.Bands().push_back( ch1);

	////////////////////////////////
	//Define Band 2 and L2 C Stream.
	Band ch2("L2cExternal");
	ch2.CenterFrequency(Frequency( 1227.6, Frequency::MHz));
	ch2.TranslatedFrequency(Frequency( 38400, Frequency::Hz));

	//Stream sm2 will be added to the global metadata
	//and as a reference to the lump.
	Stream sm2("L2C");
	sm2.RateFactor(1);
	sm2.Quantization(8);
	sm2.Packedbits(16);
	sm2.Encoding("INT8");
	sm2.Format(Stream::IQ);
	sm2.Bands().push_back(ch2);

	////////////////////////////////
	//Define the lane
	Lump lump;
	lump.Streams().push_back( sm1);
	lump.Streams().push_back( Stream( sm2.Id(), true));	


	Chunk chunk;
	chunk.SizeWord(4);
	chunk.CountWords(1);
	chunk.Lumps().push_back(lump);

	Block blk(256);
	blk.Chunks().push_back(chunk);
	
	Lane lane("GPS SPS Data");
	lane.Sessions().push_back( sess);
	lane.Blocks().push_back(blk);
	lane.AddBandSource(ch1, src1);
	lane.AddBandSource(ch2, src2);
	lane.Systems().push_back( System(sys.Id(), true));

	////////////////////////////////
	//Define the file
	File df;
	df.Url( sfile);
	df.TimeStamp( dt0);
	df.Lane( lane, true);

	//Note: we aren't specifying copyright or owner.


	////////////////////////////////
	//Assemble the Metadata object and write XML 
	Metadata md;
	XmlProcessor proc;
	md.Lanes().push_back(lane);
	md.Files().push_back( df);
	md.Systems().push_back(sys);
	md.Streams().push_back(sm2);

	try
	{
		proc.Save( sfilemd.c_str(),  md);
	}
	catch( ApiException& e)
	{
		printf("An error occurred while saving the xml file: %s\n", e.what() );
	}
}

