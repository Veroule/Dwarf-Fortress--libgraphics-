//some of this stuff is based on public domain code from nehe or opengl books over the years
//additions and modifications Copyright (c) 2008, Tarn Adams
//All rights reserved.  See game.cpp or license.txt for more information.

#ifndef ENABLER_H
#define ENABLER_H

extern "C"{
#ifndef WIN32
# include <stdint.h>
#else
# include <windows.h>
#endif
  
#include <SDL/SDL.h>
#ifdef __APPLE__
# include <SDL_image/SDL_image.h>
#else
# include <SDL/SDL_image.h>
#endif

#include "GL/glew.h"
}

#include <map>

#include "svector.h"
#include "endian.h"
#include "files.h"
#include "enabler_input.h"

#define ENABLER

#ifndef BITS

#define BITS

#define BIT1 1
#define BIT2 2
#define BIT3 4
#define BIT4 8
#define BIT5 16
#define BIT6 32
#define BIT7 64
#define BIT8 128
#define BIT9 256
#define BIT10 512
#define BIT11 1024
#define BIT12 2048
#define BIT13 4096
#define BIT14 8192
#define BIT15 16384
#define BIT16 32768
#define BIT17 65536UL
#define BIT18 131072UL
#define BIT19 262144UL
#define BIT20 524288UL
#define BIT21 1048576UL
#define BIT22 2097152UL
#define BIT23 4194304UL
#define BIT24 8388608UL
#define BIT25 16777216UL
#define BIT26 33554432UL
#define BIT27 67108864UL
#define BIT28 134217728UL
#define BIT29 268435456UL
#define BIT30 536870912UL
#define BIT31 1073741824UL
#define BIT32 2147483648UL

#endif

#define GAME_TITLE_STRING "Dwarf Fortress"

char get_slot_and_addbit_uchar(unsigned char &addbit,long &slot,long checkflag,long slotnum);

class pstringst
{
 public:
  string dat;
};

class stringvectst
{
 public:
  svector<pstringst *> str;

  void add_string(const string &st)
  {
    pstringst *newp=new pstringst;
    newp->dat=st;
    str.push_back(newp);
  }

  long add_unique_string(const string &st)
  {
    long i;
    for(i=(long)str.size()-1;i>=0;i--)
      {
	if(str[i]->dat==st)return i;
      }
    add_string(st);
    return (long)str.size()-1;
  }

  void add_string(const char *st)
  {
    if(st!=NULL)
      {
	pstringst *newp=new pstringst;
	newp->dat=st;
	str.push_back(newp);
      }
  }

  void insert_string(long k,const string &st)
  {
    pstringst *newp=new pstringst;
    newp->dat=st;
    if(str.size()>k)str.insert(k,newp);
    else str.push_back(newp);
  }

  ~stringvectst()
    {
      clean();
    }

  void clean()
  {
    while(str.size()>0)
      {
	delete str[0];
	str.erase(0);
      }
  }

  void read_file(file_compressorst &filecomp,long loadversion)
  {
    long dummy;
    filecomp.read_file(dummy);
    str.resize(dummy);

    long s;
    for(s=0;s<dummy;s++)
      {
	str[s]=new pstringst;
	filecomp.read_file(str[s]->dat);
      }
  }
  void write_file(file_compressorst &filecomp)
  {
    long dummy=str.size();
    filecomp.write_file(dummy);

    long s;
    for(s=0;s<dummy;s++)
      {
	filecomp.write_file(str[s]->dat);
      }
  }
};

class flagarrayst
{
 public:
  flagarrayst()
    {
      slotnum=0;
      array=NULL;
    }
  ~flagarrayst()
    {
      if(array!=NULL)delete[] array;
      array=NULL;
      slotnum=0;
    }

  void set_size_on_flag_num(long flagnum)
  {
    if(flagnum<=0)return;

    set_size(((flagnum-1)>>3)+1);
  }

  void set_size(long newsize)
  {
    if(newsize<=0)return;

    if(array!=NULL)delete[] array;
    array=new unsigned char[newsize];
    memset(array,0,sizeof(unsigned char)*newsize);

    slotnum=newsize;
  }

  void clear_all()
  {
    if(slotnum<=0)return;

    if(array!=NULL)memset(array,0,sizeof(unsigned char)*slotnum);
  }

  void copy_from(flagarrayst &src)
  {
    clear_all();

    if(src.slotnum>0)
      {
	set_size(src.slotnum);
	memmove(array,src.array,sizeof(unsigned char)*slotnum);
      }
  }

  bool has_flag(long checkflag)
  {
    long slot;
    unsigned char addbit;
    if(get_slot_and_addbit_uchar(addbit,slot,checkflag,slotnum))
      {
	return (array[slot]&addbit)!=0;
      }
    else return false;
  }

  void add_flag(long checkflag)
  {
    long slot;
    unsigned char addbit;
    if(get_slot_and_addbit_uchar(addbit,slot,checkflag,slotnum))
      {
	array[slot]|=addbit;
      }
  }

  void toggle_flag(long checkflag)
  {
    long slot;
    unsigned char addbit;
    if(get_slot_and_addbit_uchar(addbit,slot,checkflag,slotnum))
      {
	array[slot]^=addbit;
      }
  }

  void remove_flag(long checkflag)
  {
    long slot;
    unsigned char addbit;
    if(get_slot_and_addbit_uchar(addbit,slot,checkflag,slotnum))
      {
	array[slot]&=~addbit;
      }
  }

  void write_file(file_compressorst &filecomp)
  {
    filecomp.write_file(slotnum);
    if(slotnum>0)
      {
	long ind;
	for(ind=0;ind<slotnum;ind++)filecomp.write_file(array[ind]);
      }
  }

  void read_file(file_compressorst &filecomp,long loadversion)
  {
    long newsl;
    filecomp.read_file(newsl);
    if(newsl>0)
      {
	//AVOID UNNECESSARY DELETE/NEW
	if(array!=NULL&&slotnum!=newsl)
	  {
	    delete[] array;
	    array=new unsigned char[newsl];
	  }
	if(array==NULL)array=new unsigned char[newsl];

	long ind;
	for(ind=0;ind<newsl;ind++)filecomp.read_file(array[ind]);

	slotnum=newsl;
      }
    else if(array!=NULL)
      {
	delete[] array;
	array=NULL;
      }
  }

 private:
  unsigned char *array;
  long slotnum;
};

#ifdef ENABLER

#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_YELLOW 6
#define COLOR_WHITE	7

enum ColorData
  {
    COLOR_DATA_WHITE_R,
    COLOR_DATA_WHITE_G,
    COLOR_DATA_WHITE_B,
    COLOR_DATA_RED_R,
    COLOR_DATA_RED_G,
    COLOR_DATA_RED_B,
    COLOR_DATA_GREEN_R,
    COLOR_DATA_GREEN_G,
    COLOR_DATA_GREEN_B,
    COLOR_DATA_BLUE_R,
    COLOR_DATA_BLUE_G,
    COLOR_DATA_BLUE_B,
    COLOR_DATA_YELLOW_R,
    COLOR_DATA_YELLOW_G,
    COLOR_DATA_YELLOW_B,
    COLOR_DATA_MAGENTA_R,
    COLOR_DATA_MAGENTA_G,
    COLOR_DATA_MAGENTA_B,
    COLOR_DATA_CYAN_R,
    COLOR_DATA_CYAN_G,
    COLOR_DATA_CYAN_B,
    COLOR_DATANUM
  };

#define TILEFLAG_DEAD BIT1
#define TILEFLAG_ROTATE BIT2
#define TILEFLAG_PIXRECT BIT3
#define TILEFLAG_HORFLIP BIT4
#define TILEFLAG_VERFLIP BIT5
#define TILEFLAG_LINE BIT6
#define TILEFLAG_RECT BIT7
#define TILEFLAG_BUFFER_DRAW BIT8
#define TILEFLAG_MODEL_PERSPECTIVE BIT9
#define TILEFLAG_MODEL_ORTHO BIT10
#define TILEFLAG_MODEL_TRANSLATE BIT11
#define TILEFLAG_LINE_3D BIT12

#define TRIMAX 9999

class gridrectst
{
  friend class enablerst;

 public:
  long id;

  //FUNCTIONS
  static gridrectst *create(long newdimx,long newdimy);
  void render();

  //THE DIMENSIONS
  long dimx,dimy;

  //THE BUFFER
  long *buffer_texpos;
  float *buffer_r;
  float *buffer_g;
  float *buffer_b;
  float *buffer_br;
  float *buffer_bg;
  float *buffer_bb;
  long *s_buffer_texpos;
  float *s_buffer_r;
  float *s_buffer_g;
  float *s_buffer_b;
  float *s_buffer_br;
  float *s_buffer_bg;
  float *s_buffer_bb;
  char *s_buffer_count;

  long dispx,dispy;
  char black_space;

  //THREE-DIMENSIONAL DISPLAY
  float tri[TRIMAX][3];
  float tricol[TRIMAX][4];
  long trinum;
  float view_z,view_angle;

  void add_triangle(float t11,float t12,float t13,float tc1r,float tc1g,float tc1b,float tc1a,
		    float t21,float t22,float t23,float tc2r,float tc2g,float tc2b,float tc2a,
		    float t31,float t32,float t33,float tc3r,float tc3g,float tc3b,float tc3a);
  void allocate(long newdimx,long newdimy);
  void clean();

  void init_gl();
  void uninit_gl();

 private:
  //FUNCTIONS
  gridrectst(long newdimx,long newdimy);
  ~gridrectst();

  bool gl_initialized;
  // Arrays for vertex, texture and color data, overloaded as VBO mapping pointers in VBO mode
  bool vertices_initialized; // Used to skip rewriting them in standard and VBO mode
  GLfloat *ptr_vertex, *ptr_tex;
  GLfloat *ptr_bg_color, *ptr_fg_color;
  // True if accumulation buffer is used
  bool accum_buffer;
  // Framebuffer reference for partial-printing, or 0 if there is none
  GLuint framebuffer;
  bool framebuffer_initialized;
  GLuint fb_texture, fb_depth;
  GLuint fb_draw_list;
  // VBO references, vbo_refs[0]=0 if they are unused.
  uint32_t vbo_refs[4];
};

class text_info_elementst
{
 public:
  virtual string get_string()
  {
    string empty;
    return empty;
  }
  virtual long get_long()
  {
    return 0;
  }

  virtual ~text_info_elementst(){}
};

class text_info_element_stringst : public text_info_elementst
{
 public:
  virtual string get_string()
  {
    return str;
  }
  text_info_element_stringst(const string &newstr)
    {
      str=newstr;
    }

 protected:
  string str;
};

class text_info_element_longst : public text_info_elementst
{
 public:
  virtual long get_long()
  {
    return val;
  }
  text_info_element_longst(long nval)
    {
      val=nval;
    }

 protected:
  long val;
};

class text_infost
{
 public:
  svector<text_info_elementst *> element;

  void clean()
  {
    while(element.size()>0)
      {
	delete element[0];
	element.erase(0);
      }
  }

  string get_string(int e)
  {
    if(e<0||e>=element.size())
      {
	string empty;
	return empty;
      }
    if(element[e]==NULL)
      {
	string empty;
	return empty;
      }
    return element[e]->get_string();
  }

  long get_long(int e)
  {
    if(e<0||e>=element.size())
      {
	return 0;
      }
    if(element[e]==NULL)
      {
	return 0;
      }
    return element[e]->get_long();
  }

  ~text_infost()
    {
      clean();
    }
};

class text_system_file_infost
{
 public:
  long index;
  string filename;

  static text_system_file_infost *add_file_info(const string &newf,long newi,char newft)
  {
    return new text_system_file_infost(newf,newi,newft);
  }

  void initialize_info();
  void get_text(text_infost &text);
  void get_specific_text(text_infost &text,long num);

 protected:
  char file_token;
  long number;

  text_system_file_infost(const string &newf,long newi,char newft)
    {
      filename=newf;
      file_token=newft;
      index=newi;
      number=0;
    }
};

class text_systemst
{
 public:
  void register_file_fixed(const string &file_name,long index,char token,char initialize)
  {
    text_system_file_infost *tsfi=text_system_file_infost::add_file_info(file_name,index,token);
    if(initialize)tsfi->initialize_info();
    file_info.push_back(tsfi);
  }
  void register_file(const string &file_name,long &index,char token,char initialize)
  {
    long t;
    for(t=(long)file_info.size()-1;t>=0;t--)
      {
	if(file_info[t]->filename==file_name)
	  {
	    //RESET CALLING INDEX AND BAIL IF THIS FILE IS ALREADY IN THE SYSTEM
	    index=file_info[t]->index;
	    return;
	  }
      }

    text_system_file_infost *tsfi=text_system_file_infost::add_file_info(file_name,index,token);
    if(initialize)tsfi->initialize_info();
    file_info.push_back(tsfi);
  }
  void initialize_system()
  {
    long t;
    for(t=(long)file_info.size()-1;t>=0;t--)file_info[t]->initialize_info();
  }
  void get_text(long index,text_infost &text)
  {
    long t;
    for(t=(long)file_info.size()-1;t>=0;t--)
      {
	if(file_info[t]->index==index)
	  {
	    file_info[t]->get_text(text);
	    return;
	  }
      }
  }
  void get_text(const string &file_name,text_infost &text)
  {
    long t;
    for(t=(long)file_info.size()-1;t>=0;t--)
      {
	if(file_info[t]->filename==file_name)
	  {
	    file_info[t]->get_text(text);
	    return;
	  }
      }
  }
  void get_specific_text(long index,text_infost &text,long num)
  {
    long t;
    for(t=(long)file_info.size()-1;t>=0;t--)
      {
	if(file_info[t]->index==index)
	  {
	    file_info[t]->get_specific_text(text,num);
	    return;
	  }
      }
  }

  ~text_systemst()
    {
      while(file_info.size()>0)
	{
	  delete file_info[0];
	  file_info.erase(0);
	}
    }

 protected:
  svector<text_system_file_infost *> file_info;
};

class curses_text_boxst
{
	public:
		stringvectst text;

		void add_paragraph(stringvectst &src,long para_width);
		void add_paragraph(const string &src,long para_width);

		void read_file(file_compressorst &filecomp,long loadversion)
			{
			text.read_file(filecomp,loadversion);
			}
		void write_file(file_compressorst &filecomp)
			{
			text.write_file(filecomp);
			}
		void clean()
			{
			text.clean();
			}
};

#define COPYTEXTUREFLAG_HORFLIP BIT1
#define COPYTEXTUREFLAG_VERFLIP BIT2

#define ENABLERFLAG_RENDER BIT1
#define ENABLERFLAG_MAXFPS BIT2

// GL texture positions
struct gl_texpos {
  GLfloat left, right, top, bottom;
};

// Being a texture catalog interface, with opengl and sdl capability
class textures
{
  friend class gridrectst;
  friend class enablerst;
 private:
  std::vector<SDL_Surface *> raws;
  bool uploaded;
  long add_texture(SDL_Surface*);
 protected:
  GLuint gl_catalog; // texture catalog gennum
  struct gl_texpos *gl_texpos; // Texture positions in the GL catalog, if any
 public:
  // Initialize state variables
  textures() {
    uploaded=false;
    gl_texpos = NULL;
  }
  int textureCount() {
    return raws.size();
  }
  // Upload in-memory textures to the GPU, or delete them
  // When textures are uploaded, any alteration to a texture
  // is automatically reflected in the uploaded copy - eg. it's replaced.
  // This is very expensive in opengl mode. Don't do it often.
  void upload_textures();
  // Also, you really should make sure to remove uploaded textures before
  // deleting a window.
  void remove_uploaded_textures();
  // Returns the most recent texture data
  SDL_Surface *get_texture_data(long pos);
  // Clone a texture
  long clone_texture(long src);
  // Remove all color, but not transparency
  void grayscale_texture(long pos);
  // Loads dimx*dimy textures from a file, assuming all tiles
  // are equally large and arranged in a grid
  // Texture positions are saved in row-major order to tex_pos
  // If convert_magenta is true and the file does not have built-in transparency,
  // any magenta (255,0,255 RGB) is converted to full transparency
  // The calculated size of individual tiles is saved to disp_x, disp_y
  void load_multi_pdim(const string &filename,long *tex_pos,long dimx,long dimy,
		       bool convert_magenta,
/* 		       long *adj_x, long *adj_y, */
		       long *disp_x, long *disp_y);
  // Loads a single texture from a file, returning the handle
  long load(const string &filename, bool convert_magenta);
/*   SDL_Surface *get_texture_data(long pos); */
/*   SDL_Surface *load_bitmap_file(const string &filename); */
/*   SDL_Surface *load_bitmap_file_with_alpha(const string &filename); */
  // To delete a texture..
  void delete_texture(long pos);
};

struct tile {
  int x, y;
  long tex;
};

typedef struct {									// Window Creation Info
  char*				title;						// Window Title
  int					width;						// Width
  int					height;						// Height
  int					bitsPerPixel;				// Bits Per Pixel
  BOOL				isFullScreen;				// FullScreen?
} GL_WindowInit;									// GL_WindowInit

typedef struct {									// Contains Information Vital To A Window
  GL_WindowInit		init;						// Window Init
  BOOL				isVisible;					// Window Visible?
} GL_Window;										// GL_Window

class enablerst: public enabler_inputst
{
  friend class gridrectst;
  friend class tilest;
  friend class textures;

 public:
  GL_Window			window;											// Window Structure
  BOOL				isMessagePumpActive;							// Message Pump Active?
  MSG					msg;											// Window Message Structure

  int desired_windowed_width,desired_windowed_height;
  int desired_fullscreen_width,desired_fullscreen_height;
  unsigned long flag;
  char tracking_on;

  string command_line;


  int loop(void);

  void terminate_application(GL_Window* window);
  char create_full_screen;
  char inactive_mode;
  void reshape_GL(int width,int height);
  void render(GL_Window &window);
  void render_tiles();
  void graphicsinit();
  long gridrect_create(long dimx,long dimy);
  long cursesrect_create(long font_id,long x,long y,long dimx,long dimy);
/*   long font_create(const string &filename,long letx,long lety,char variable=0); */
  gridrectst *get_gridrect(long rect_id);
  void toggle_fullscreen()
  {
    toggle_fullscreen(&window);
  }
/*   void locate(double fx,double fy,double fz=0); */
/*   void add_perspective(float sx,float sy,float sz, */
/* 		       float gx,float gy,float gz, */
/* 		       float nx,float ny,float nz); */
/*   void add_translate(float sx,float sy,float sz); */
/*   void add_ortho(); */
/*   void add_line(double endx,double endy); */
/*   void add_line_3D(double endx,double endy,double endz); */
/*   void add_rect(double endx,double endy); */
  // Display a particular tile at a particular window-relative location
  // Pass -1 as tex_pos to remove a particular tile. Using the same id twice
  // overwrites the last call.
  void set_tile(long tex_pos, int id, int x, int y);
  void add_gennum_tile(long gennum,double dimx,double dimy,double letx,double lety,char alpha);
  void set_color(float r,float g,float b,float a=1);
  void enable_fade(float r,float g,float b,float a,float t);
  void disable_fade();
  void set_font(long font_id)
  {
    active_font_id=font_id;
  }
  void print_string(const string &str,char centered=0,short length_lim=1000,char crammed_lets=0);
  void set_center(double newx,double newy)
  {
    center_x=newx;
    center_y=newy;
  }
  void set_clear_color(float newr,float newg,float newb,float newa)
  {
    if(fade_t==0)
      {
	clear_r=newr;clear_g=newg;clear_b=newb;clear_a=newa;
      }
    else
      {
	clear_r=newr*(1.0f-fade_t)+fade_r*fade_t;
	clear_g=newg*(1.0f-fade_t)+fade_g*fade_t;
	clear_b=newb*(1.0f-fade_t)+fade_b*fade_t;
	clear_a=newa*(1.0f-fade_t)+fade_a*fade_t;
      }
  }
  unsigned long get_timer()
  {
    if(main_qprate.QuadPart==0)return 0;
    else return qpc.QuadPart;
  }

void copy_texture_data(unsigned char *dest,long destx,long desty,char destalpha,
			unsigned char *src,long srcx,long srcy,char srcalpha,
			long offx,long offy,
			float rmult,float gmult,float bmult,char use_trans,long *color_data=NULL,unsigned long flag=0);

void save_texture_data_to_bmp(unsigned char *bitmapImage,long dimx,long dimy,long alpha,string &filename);


/*   void copy_texture_data(long dest,long src,long offx=0,long offy=0, */
/* 			 float rmult=1,float gmult=1,float bmult=1,char use_trans=1,long *color_data=NULL,unsigned long flag=0); */
/*   void copy_texture_data(long dest,unsigned char *src,long srcx,long srcy,char srcalpha, */
/* 			 long offx=0,long offy=0, */
/* 			 float rmult=1,float gmult=1,float bmult=1,char use_trans=1,long *color_data=NULL,unsigned long flag=0); */
/*   void grayscale_texture_data(long pos); */
/*   void antialias(unsigned char *dat,long srcx,long srcy,char border=0); */
  void flip_uchar_array(unsigned char *buff,long dimx,long dimy,long bytes_per_pixel,unsigned long flag);
  void get_texture_data_dims(long pos,short &width,short &height,char &alpha);
/*   { */
/*     if(pos<0||pos>=texture_data.size())return; */

/*     if(texture_data[pos]==NULL)return; */

/*     width=texture_data[pos]->width; */
/*     height=texture_data[pos]->height; */
/*     alpha=texture_data[pos]->alpha_texture; */
/*   } */
/*   void remove_texture(long pos); */
/*   void remove_texture_data(long pos); */
/*   void force_texture_create(long pos); */
/*   { */
/*     if(pos<0||pos>=texture.size())return; */

/*     if(texture[pos]==NULL)return; */

/*     texture[pos]->remove_texture(); */
/*     texture[pos]->create_texture(); */
/*   } */

  void read_pixels(int x,int y,int width,int height,unsigned char *buffer);

/*   void save_texture_data_to_bmp(long pos,string &filename); */

  enablerst();
  ~enablerst()
    {
      long t;
      for(t=(long)gridrect.size()-1;t>=0;t--)
	{
	  delete gridrect[t];
	}
      gridrect.clear();
/*       for(t=(long)texture.size()-1;t>=0;t--) */
/* 	{ */
/* 	  if(texture[t]!=NULL)delete texture[t]; */
/* 	} */
/*       texture.clear(); */
/*       for(t=(long)texture_data.size()-1;t>=0;t--) */
/* 	{ */
/* 	  if(texture_data[t]!=NULL)delete texture_data[t]; */
/* 	} */
/*       texture_data.clear(); */
    }
  void enable_buffer_draw(){buffer_draw=1;}
  void disable_buffer_draw(){buffer_draw=0;}
  char doing_buffer_draw(){return buffer_draw;}

  LARGE_INTEGER qpfr;
  LARGE_INTEGER main_qprate,qprate,g_qprate;
  LARGE_INTEGER qpc,qpc2,g_qpc;
		
  double frames_outstanding;
  SDL_cond *timer_cond;
  SDL_mutex *dummy_mutex;

  Uint32 now; //we only need to be getting the tick time once

  unsigned long current_render_count;
  unsigned long secondary_render_count;

  long oldmouse_x,oldmouse_y,mouse_x,mouse_y;
  char mouse_lbut,mouse_rbut;
  char mouse_lbut_down,mouse_rbut_down;
  char mouse_lbut_lift,mouse_rbut_lift;

  text_systemst text_system;
  char change_screen_resolution (int width, int height, int bitsPerPixel);

  void refresh_tiles();

  float ccolor[16][3];
  int window_width,window_height;

  void do_frame();
  class textures textures;

 private:
  long next_tile_slot;
  long active_font_id;

  svector<enabler_inputst> input;
  char is_program_looping;
  svector<gridrectst *> gridrect;
  long next_gridrect_id;
  long next_texture_id;
  long next_texture_data_id;
  double locx,locy,locz;
  double center_x,center_y;
  float color_r,color_g,color_b,color_a;
  float clear_r,clear_g,clear_b,clear_a;
  float fade_r,fade_g,fade_b,fade_a,fade_t;
  char buffer_draw;

  // Tile-map used by set_tile
  std::map<int,struct tile> tiles;
  // Vertex and texture arrays generated from above map
  GLfloat *tile_vertices;
  GLfloat *tile_texcoords;


  char create_window_GL (GL_Window* window);
  char destroy_window_GL (GL_Window* window);
  char register_window_class (void);
  void toggle_fullscreen(GL_Window* window);
  void create_textures();
  void remove_textures();
/*   { */
/*     long t; */
/*     for(t=(long)texture.size()-1;t>=0;t--) */
/*       { */
/* 	if(texture[t]==NULL)continue; */

/* 	texture[t]->remove_texture(); */
/*       } */
/*   } */

 public:
  void reset_gl(GL_Window* window);
  void reset_gl()
  {
    reset_gl(&window);
  }
};

void convert_to_rgb(float &r,float &g,float &b,char col,char bright);
#endif

#endif //ENABLER_H
