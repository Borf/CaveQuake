/* Aftershock 3D rendering engine
 * Copyright (C) 1999 Stephen C. Taylor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "util.h"
#include "pak.h"
#include "bsp.h"
#include "shader.h"
#include "tex.h"
#include "glinc.h"
#include <string.h>
#include <stdio.h>

void setStatus(const char*,...);


#include "libjpeg/jpeglib.h"


#include "uicommon.h"

#define IMG_BUFSIZE (1024*1024-8)

#include "globalshared.h"
extern global_shared_t *g;

typedef struct
{
    byte_t idlen;
    byte_t cmtype;
    byte_t imgtype;
    byte_t cmspec[5];
    ushort_t xorig, yorig;
    ushort_t width, height;
    byte_t pixsize;
    byte_t imgdesc;
} tgaheader_t;

static int tga_readtex(const char *fname, byte_t **rgb, int *w, int *h,
		       int *format);
static int jpg_readtex(const char *fname, byte_t **rgb, int *w, int *h,
		       int *format);
static void tex_loadtexture(byte_t *rgb, int w, int h, int format, uint_t flags);

static void *imgbuf;

static void showProgress(float curr, float total) {
  int i;
  int val = (int) (curr/total * 50);
  printf(" Loading |");
  for (i=0; i<=50; i++) {
    if (i <= val)
      printf("#");
    else
      printf("-");
  }
  printf("| %d%%\r", val*2);
  setStatus("CaveQuake\nLoading textures (%.2f%%) %.0f of %.0f", curr/total * 100.0, curr, total);
  fflush(stdout);
}

void
tex_loadall(void)
{
    int i;
    byte_t *rgb;
    int width, height, format, len, size;

    if (g->r_notextures)
      return;

    g->r_texture_data = (tex_data_t *) gc_malloc(g->r_numtextures * sizeof(tex_data_t));
    
    imgbuf = malloc(IMG_BUFSIZE);
   
    for (i=0; i < g->r_numtextures; ++i) {

        rgb = NULL;
        showProgress(i, g->r_numtextures-1);
	/* printf("loading...%s\n", g->r_texfiles[i].fname);  */

	len = strlen(g->r_texfiles[i].fname);
	if (!strcmp(&(g->r_texfiles[i].fname[len-4]), ".jpg"))
	{
	    if (!jpg_readtex(g->r_texfiles[i].fname, &rgb, &width, &height,
			     &format))
		Error("Could not open file %s", g->r_texfiles[i].fname);
		/* continue; */
                i = i; /* nothing */
	}
	else if (!strcmp(&(g->r_texfiles[i].fname[len-4]), ".tga"))
	{
	    if (!tga_readtex(g->r_texfiles[i].fname, &rgb, &width, &height,
			     &format))
	    {
		/* Might still be a jpg file !!! (compatibility with old
		   shader scripts?) */
		strcpy(&(g->r_texfiles[i].fname[len-3]), "jpg");
		if (!jpg_readtex(g->r_texfiles[i].fname, &rgb, &width,
				 &height, &format))
		{
		    /* FIXME: This should be an error, but still happens
		       in the demo levels ! */
		    strcpy(&(g->r_texfiles[i].fname[len-3]), "tga");
		    printf("Could not open file %s\n", g->r_texfiles[i].fname);
		    /* continue; */
		}
	    }
	}
	else {
	    Error("Unknown format for %s", g->r_texfiles[i].fname);
		/* continue;
 */
	}

        size = width*height* (format == GL_RGB ? 3 : 4);

        /* Not really a gamma: prelighten the texture to compensate for
           darkening after lightmap application. */
        if (rgb && (g->r_gamma != 1.0 || g->r_brightness != 0))
        {
    	    int i, val;
	    for (i=0; i<size; ++i)
	    {
	        val = (rgb[i] + g->r_brightness) * g->r_gamma;
                /*val = gammaCorrect(rgb[i], g->r_gamma);*/
	        if (val > 255) val = 255;
	        rgb[i] = val;
	    }
        }
	g->r_texture_data[i].rgb = rgb;
	g->r_texture_data[i].w = width;
	g->r_texture_data[i].h = height;
	g->r_texture_data[i].format = format;
    }
    free(imgbuf);
}

void tex_bindobjs(r_context_t *c) {
  int i;
  tex_data_t tdata;

  c->r_textures = (uint_t*) rc_malloc(g->r_numtextures * sizeof(uint_t));
  glGenTextures(g->r_numtextures, c->r_textures);

  if (g->r_notextures)
    return;

  for (i=0; i < g->r_numtextures; i++) {
    tdata = g->r_texture_data[i];
    if (tdata.rgb != NULL) {
      glBindTexture(GL_TEXTURE_2D, c->r_textures[i]);
      tex_loadtexture(tdata.rgb, tdata.w, tdata.h, tdata.format, g->r_texfiles[i].flags);
    }
  }
}

void tex_freeall() {
  int i;

  for (i=0; i<g->r_numtextures; i++) {
    tex_data_t tdata = g->r_texture_data[i];
    gc_free(tdata.rgb);
  }
  gc_free(g->r_texture_data);
}

void
tex_freeobjs(r_context_t *c)
{
    glDeleteTextures(g->r_numtextures, c->r_textures);
    rc_free(c->r_textures);
}

/* 
static int gammaCorrect(int intensity, float gamma) {
  return (int) (pow((intensity/255.0f), 1.0f/gamma) * 255);
}
*/

static void
tex_loadtexture(byte_t *rgb, int w, int h, int format, uint_t flags)
{
    byte_t *tex = rgb;
    int width = w, height = h;
    int size = width*height* (format == GL_RGB ? 3 : 4);

    /* Scale image down for biased level of detail (lowered texture quality) */
    if (!(flags & TEXFILE_FULL_LOD) && g->r_lodbias > 0)
    {
	width /= 1 << g->r_lodbias;
	height /= 1 << g->r_lodbias;
        tex = (byte_t *) rc_malloc(size);

        gluScaleImage(format, w, h, GL_UNSIGNED_BYTE, rgb,
		      width, height, GL_UNSIGNED_BYTE, tex);
    }


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
	    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (flags & TEXFILE_CLAMP)
    {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    else
    {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (flags & TEXFILE_NOMIPMAPS)
    {
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
		     GL_UNSIGNED_BYTE, tex);
    }
    else
    {
	gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format,
			  GL_UNSIGNED_BYTE, tex);
    }

    if (g->r_lodbias > 0)
      free(tex);
}

static int
tga_readtex(const char *fname, byte_t **rgb, int *w, int *h, int *format)
{
    tgaheader_t *tgahead;
    byte_t *img, *tga, *tgacur, *tgaend;
    int tgalen, len, depth = 0;

    tgalen = pak_readfile(fname, IMG_BUFSIZE, (byte_t *) imgbuf);
    if (!tgalen) return 0;

    tga = (byte_t*)imgbuf;
    tgaend = tga + tgalen;
    
    tgahead = (tgaheader_t*)tga;
    BYTESWAPSHORT(tgahead->xorig);
    BYTESWAPSHORT(tgahead->yorig);
    BYTESWAPSHORT(tgahead->width);
    BYTESWAPSHORT(tgahead->height);
    if (tgahead->imgtype != 2 && tgahead->imgtype != 10) {
        fprintf(stderr, "Bad tga image type\n");
        return 0;
	//Error("Bad tga image type");
    }	 

    if (tgahead->pixsize == 24)
	depth = 3;
    else if (tgahead->pixsize == 32)
	depth = 4;
    else
	Error("Non 24 or 32 bit tga image");
    
    len = tgahead->width * tgahead->height * depth;
    /* img = (byte_t *) gc_malloc(len); */
    /* no need to share */
    img = (byte_t *) malloc(len);

    tgacur = tga + sizeof(tgaheader_t) + tgahead->idlen;
    if (tgahead->imgtype == 10)
    {
	int i, j, packetlen;
	byte_t packethead;
	byte_t *c = img, *end = img + len;
	byte_t rlc[4];
	
	while (c < end)
	{
	    packethead = *tgacur;
	    if (++tgacur > tgaend)
		Error("Unexpected end of tga file");
	    if (packethead & 0x80)
	    {
		/* Run-length packet */
		packetlen = (packethead & 0x7f) + 1;
		memcpy(rlc, tgacur, depth);
		if ((tgacur += depth) > tgaend)
		    Error("Unexpected end of tga file");
		for (j=0; j < packetlen; ++j)
		    for(i=0; i < depth; ++i)
			*c++ = rlc[i];
	    }
	    else
	    {
		/* Raw data packet */
		packetlen = packethead + 1;
		memcpy(c, tgacur, depth * packetlen);
		if ((tgacur += depth * packetlen) > tgaend)
		    Error("Unexpected end of tga file");
		c += packetlen * depth;
	    }
	}

	/* Flip image in y */
	{
	    int i, linelen;
	    byte_t *temp;
	    
	    linelen = tgahead->width * depth;
	    temp = (byte_t *) malloc(linelen);
	    for (i=0; i < tgahead->height/2; ++i)
	    {
		memcpy(temp, &img[i * linelen], linelen);
		memcpy(&img[i * linelen], &img[(tgahead->height - i - 1)
					      * linelen], linelen);
		memcpy(&img[(tgahead->height - i - 1) * linelen], temp,
		       linelen);
	    }
	    free(temp);
	}	
    }
    else
    {
	int i, linelen;
	
	if (tgaend - tgacur + 1 < len)
	    Error("Bad tga image data length");

	/* Flip image in y */
	linelen = tgahead->width * depth;
	for (i=0; i < tgahead->height; ++i)
	    memcpy(&img[i * linelen],
		   &tgacur[(tgahead->height - i - 1) * linelen], linelen);
    }    

    /* Exchange B and R to get RGBA ordering */
    {
	int i;
	byte_t temp;

	for (i=0; i < len; i += depth)
	{
	    temp = img[i];
	    img[i] = img[i+2];
	    img[i+2] = temp;
	}
    }
    
    *rgb = img;
    *w = tgahead->width;
    *h = tgahead->height;
    *format = (depth == 3) ? GL_RGB : GL_RGBA;

/*	free(imgbuf); */
    return 1;
}

static void
jpg_noop(j_decompress_ptr cinfo)
{
}

static boolean
jpg_fill_input_buffer(j_decompress_ptr cinfo)
{
    Error("Premeture end of jpeg file");
    return TRUE;
}

static void
jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
        
    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
    if (cinfo->src->bytes_in_buffer < 0)
	Error("Premeture end of jpeg file");
}

static void
jpeg_mem_src(j_decompress_ptr cinfo, byte_t *mem, int len)
{
    cinfo->src = (struct jpeg_source_mgr *)
	(*cinfo->mem->alloc_small)((j_common_ptr) cinfo,
				   JPOOL_PERMANENT,
				   sizeof(struct jpeg_source_mgr));
    cinfo->src->init_source = jpg_noop;
    cinfo->src->fill_input_buffer = jpg_fill_input_buffer;
    cinfo->src->skip_input_data = jpg_skip_input_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart;
    cinfo->src->term_source = jpg_noop;
    cinfo->src->bytes_in_buffer = len;
    cinfo->src->next_input_byte = mem;
}

static int
jpg_readtex(const char *fname, byte_t **rgb, int *w, int *h, int *format)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    byte_t *img, *c;
    int jpglen;

    jpglen = pak_readfile(fname, IMG_BUFSIZE, (byte_t *) imgbuf);
    if (!jpglen) return 0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, (byte_t *) imgbuf, jpglen);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    if (cinfo.output_components != 3)
	Error("Bad number of jpg components");

    /* img = c = (byte_t *) gc_malloc(cinfo.output_width * cinfo.output_height * 3); */
    /* no need to share */
    img = c = (byte_t *) malloc(cinfo.output_width * cinfo.output_height * 3);
    while (cinfo.output_scanline < cinfo.output_height)
    {
	jpeg_read_scanlines(&cinfo, &c, 1);
	c += cinfo.output_width * 3;
    }

    *rgb = img;
    *w = cinfo.output_width;
    *h = cinfo.output_height;
    *format = GL_RGB;

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return 1;
}
