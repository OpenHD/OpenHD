/*
* Copyright (c) 2006, Hybrid Graphics, Ltd.
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Hybrid Graphics may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY HYBRID GRAPHICS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL HYBRID GRAPHICS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <float.h>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#define OUTPUT_INTS
#define NGLYPHS 500
class Vector2
{
public:
	Vector2()						{}
	Vector2(float px, float py)		{ x = px; y = py; }
	
	float		x;
	float		y;
};

Vector2	operator+(const Vector2& a, const Vector2& b)	{ return Vector2(a.x+b.x, a.y+b.y); }
Vector2	operator*(const Vector2& a, float b)	{ return Vector2(a.x*b, a.y*b); }

float convFTFixed( const FT_Pos &x )
{
	return (float)x / 4096.0f;
}

Vector2 convFTVector( const FT_Vector &v )
{
	return Vector2(convFTFixed(v.x),convFTFixed(v.y));
}

bool isOn( char b )
{
	return b & 1 ? true : false;
}

int main (int argc, char * const argv[])
{
	FT_Library library;
	FT_Face face;

	if(argc < 4)
	{
		printf("usage: font2openvg input_font_file output.c prefix\n");
		exit(-1);
	}

	if( FT_Init_FreeType( &library ) )
	{
		printf("couldn't initialize freetype\n");
		exit(-1);
	}
	int faceIndex = 0;
	if( FT_New_Face( library, argv[1], faceIndex, &face ) )
	{
		printf("couldn't load new face\n");
		exit(-1);
	}

	FT_Set_Char_Size(
              face,    /* handle to face object           */
              0,       /* char_width in 1/64th of points  */
              64*64,   /* char_height in 1/64th of points */
              96,     /* horizontal device resolution    */
              96 );   /* vertical device resolution      */

	FILE* f = fopen(argv[2], "wt");
	if(!f)
	{
		printf("couldn't open %s for writing\n", argv[2]);
		exit(-1);
	}

	std::vector<int>		gpvecindices;
	std::vector<int>		givecindices;
	std::vector<int>		gpvecsizes;
	std::vector<int>		givecsizes;
	std::vector<Vector2>	gpvec;
	std::vector<char>		givec;
	std::vector<float>		gbbox;
	std::vector<float>		advances;

        float                           global_miny = 1000000.0f;
        float                           global_maxy = -10000000.0f;
        
	unsigned int characterMap[NGLYPHS];
	int glyphs = 0;
	for(int cc=0;cc<NGLYPHS;cc++)
	{
		characterMap[cc] = 0xffffffffu;	//initially nonexistent

		if( cc < 32 )
			continue;	//discard the first 32 characters

		int glyphIndex = FT_Get_Char_Index( face, cc );

		if( !FT_Load_Glyph( face, glyphIndex, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_IGNORE_TRANSFORM ) )
		{
			float advance = convFTFixed( face->glyph->advance.x );
			if( cc == ' ' )
			{	//space doesn't contain any data
				gpvecindices.push_back( gpvec.size() );
				givecindices.push_back( givec.size() );

				gpvecsizes.push_back( 0 );
				givecsizes.push_back( 0 );

				gbbox.push_back(0);
				gbbox.push_back(0);
				gbbox.push_back(0);
				gbbox.push_back(0);

				advances.push_back(advance);

				//write glyph index to character map
				characterMap[cc] = glyphs++;
				continue;
			}

			FT_Outline &outline = face->glyph->outline;
			std::vector<Vector2>		pvec;
			std::vector<unsigned char>	ivec;
			float minx = 10000000.0f,miny = 100000000.0f,maxx = -10000000.0f,maxy = -10000000.0f;
			int s = 0,e;
			bool on;
			Vector2 last,v,nv;
			for(int con=0;con<outline.n_contours;++con)
			{
				int pnts = 1;
				e = outline.contours[con]+1;
				last = convFTVector(outline.points[s]);

				//read the contour start point
				ivec.push_back(2);
				pvec.push_back(last);

				int i=s+1;
				while(i<=e)
				{
					int c = (i == e) ? s : i;
					int n = (i == e-1) ? s : (i+1);
					v = convFTVector(outline.points[c]);
					on = isOn( outline.tags[c] );
					if( on )
					{	//line
						++i;
						ivec.push_back(4);
						pvec.push_back(v);
						pnts += 1;
					}
					else
					{	//spline
						if( isOn( outline.tags[n] ) )
						{	//next on
							nv = convFTVector( outline.points[n] );
							i += 2;
						}
						else
						{	//next off, use middle point
							nv = (v + convFTVector( outline.points[n] )) * 0.5f;
							++i;
						}
						ivec.push_back(10);
						pvec.push_back(v);
						pvec.push_back(nv);
						pnts += 2;
					}
					last = nv;
				}
				ivec.push_back(0);
				s = e;
			}

			for(int i=0;i<pvec.size();++i)
			{
				if( pvec[i].x < minx ) minx = pvec[i].x;
				if( pvec[i].x > maxx ) maxx = pvec[i].x;
				if( pvec[i].y < miny ) miny = pvec[i].y;
				if( pvec[i].y > maxy ) maxy = pvec[i].y;
			}
			if(!pvec.size())
			{
				minx = 0.0f;
				miny = 0.0f;
				maxx = 0.0f;
				maxy = 0.0f;
			}

			gpvecindices.push_back( gpvec.size() );
			givecindices.push_back( givec.size() );

			gpvecsizes.push_back( pvec.size() );
			givecsizes.push_back( ivec.size() );

			gbbox.push_back( minx );
			gbbox.push_back( miny );
			gbbox.push_back( maxx );
			gbbox.push_back( maxy );
			advances.push_back(advance);

                        if (miny < global_miny) {
                            global_miny = miny;
                        }
                        if (maxy > global_maxy) {
                            global_maxy = maxy;
                        }
                        
			int size;
			size = gpvec.size();
			gpvec.resize( size + pvec.size() );
			memcpy( &(gpvec[size]), &(pvec[0]), pvec.size() * sizeof(Vector2) );

			size = givec.size();
			givec.resize( size + ivec.size() );
			memcpy( &(givec[size]), &(ivec[0]), ivec.size() * sizeof(char) );

			//write glyph index to character map
			characterMap[cc] = glyphs++;
		}
	}
	if(!glyphs)
		printf("warning: no glyphs found\n");

	static const char* legalese = {"/* Generated by font2openvg. See http://developer.hybrid.fi for more information. */\n\n"};

	//print legalese
	fprintf(f,"%s", legalese);


	//print the name of the font file
	fprintf (f,"/* converted from font file %s */\n", argv[1]);
	fprintf (f,"/* font family name: %s */\n", face->family_name);
	fprintf (f,"/* font style name: %s */\n\n", face->style_name);

	//print instructions
	fprintf (f,"static const unsigned char %s_glyphInstructions[%d] = {", argv[3],givec.size());
	for(int i=0;i<givec.size();i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%d%c",givec[i],(i==(givec.size()-1))?' ':',');
	}
	fprintf (f,"};\n");

	fprintf (f,"static const int %s_glyphInstructionIndices[%d] = {", argv[3],givecindices.size());
	for(int i=0;i<givecindices.size();i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%d%c",givecindices[i],(i==(givecindices.size()-1))?' ':',');
	}
	fprintf (f,"};\n");

	fprintf (f,"static const int %s_glyphInstructionCounts[%d] = {", argv[3],givecsizes.size());
	for(int i=0;i<givecsizes.size();i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%d%c",givecsizes[i],(i==(givecsizes.size()-1))?' ':',');
	}
	fprintf (f,"};\n\n");

	fprintf (f,"static const int %s_glyphPointIndices[%d] = {", argv[3],gpvecindices.size());
	for(int i=0;i<gpvecindices.size();i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%d%c",gpvecindices[i],(i==(gpvecindices.size()-1))?' ':',');
	}
	fprintf (f,"};\n");

#ifdef OUTPUT_INTS
	//print points
	fprintf (f,"static const int %s_glyphPoints[%d*2] = {", argv[3],gpvec.size());
	for(int i=0;i<gpvec.size();i++)
	{
		if ((i % 10)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%d,%d%c",(int)(65536.0f*gpvec[i].x),(int)(65536.0f*gpvec[i].y),(i==(gpvec.size()-1))?' ':',');
	}
	fprintf (f,"};\n");

	//print the advances
	fprintf (f,"static const int %s_glyphAdvances[%d] = {", argv[3],advances.size());
	for(int i=0;i<advances.size();i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%d%c",(int)(65536.0f*advances[i]),(i==(advances.size()-1))?' ':',');
	}
	fprintf (f,"};\n\n");

	//print the bounding boxes
/*
	fprintf (f,"static const int %s_glyphBBoxes[%d] = {", argv[3],gbbox.size());
	for(int i=0;i<gbbox.size();i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%d%c",(int)(65536.0f*gbbox[i]),(i==(gbbox.size()-1))?' ':',');
	}
	fprintf (f,"};\n\n");
*/
        //print minimum and maximum y values over the whole font
        fprintf (f,"static const int %s_descender_height = %d;\n",argv[3],(int)(65536.0f*global_miny));
        fprintf (f,"static const int %s_ascender_height = %d;\n",argv[3],(int)(65536.0f*global_maxy));
#else
	//print points
	fprintf (f,"static const float %s_glyphPoints[%d*2] = {", argv[3],gpvec.size());
	for(int i=0;i<gpvec.size();i++)
	{
		if ((i % 10)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%f,%f%c",gpvec[i].x,gpvec[i].y,(i==(gpvec.size()-1))?' ':',');
	}
	fprintf (f,"};\n");

	//print the advances
	fprintf (f,"static const float %s_glyphAdvances[%d] = {", argv[3],advances.size());
	for(int i=0;i<advances.size();i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%f%c",advances[i],(i==(advances.size()-1))?' ':',');
	}
	fprintf (f,"};\n\n");

	//print the bounding boxes
/*
	fprintf (f,"static const float %s_glyphBBoxes[%d] = {", argv[3],gbbox.size());
	for(int i=0;i<gbbox.size();i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%f%c",gbbox[i],(i==(gbbox.size()-1))?' ':',');
	}
	fprintf (f,"};\n\n");
*/
        fprintf (f,"static const float %s_descender_height = %d;\n",argv[3],global_miny));
        fprintf (f,"static const float %s_ascender_height = %d;\n",argv[3],global_maxy));
#endif


	//print the number of glyphs and the character map
	fprintf (f,"static const int %s_glyphCount = %d;\n",argv[3],glyphs);
	fprintf (f,"static const short %s_characterMap[500] = {", argv[3]);
	for(int i=0;i<NGLYPHS;i++)
	{
		if ((i % 20)==0)
			fprintf (f,"\n    ");
		fprintf (f,"%d%c",characterMap[i],(i==(NGLYPHS-1))?' ':',');
	}
	fprintf (f,"};\n\n");
	fclose(f);

	if(glyphs)
		printf("%d glyphs written\n", glyphs);

	FT_Done_Face( face );
	FT_Done_FreeType( library );
    return 0;
}
