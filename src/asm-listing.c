/*
  Copyright (C) 2003  Sridhar Ratnakumar <srid@nearfar.org>
	
  This file is part of GNUSim8085.

  GNUSim8085 is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  GNUSim8085 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GNUSim8085; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "asm-listing.h"
#include "asm-ds-limits.h"

static GString *
get_hex_list (const gchar * str, gint * total)
{
  gchar **slist, **p;
  GString *r;
  gint val;
  g_assert (str);
  g_assert (total);

  /* split */
  slist = g_strsplit (str, ",", -1);
  r = g_string_new (" ");
	
  /* scan */
  p = slist;
  *total = 0;
  while (*p)
	{
	  if ( !asm_source_entry_parse_not_operand_but_this(NULL, &val, *p) )
		{
		  g_string_free (r, TRUE);
		  r = NULL;
		  break;
		}
	  (*total)++;
		
	  /* add hex */
	  if (val > 0x0f)
		g_string_append_printf (r, " %02X", val);
	  else
		g_string_append_printf (r, " 0%X", val);
	  p++;
	}
	
  g_strfreev (slist);
  return r;
}


/* get the listing of bin codes */
void
asm_listing_get_binary_array (AsmSource *src, GString **listing)
{
  int entry_i, listing_i = 0;
	
  for (entry_i =0; entry_i<src->entries_total; entry_i++ )
	{
	  AsmSourceEntry *entry = src->entries[entry_i];
	  gint pto;
	  GString *line;
		
		
	  g_assert (entry);
	  pto = entry->listing_buffer_line_no;
	  listing[pto] = g_string_new ("");
		
	  g_assert (pto>=listing_i);
	  /* write listing before this entry */
	  while ( pto > listing_i )
		{
		  line = src->listing_buffer[listing_i];
		  g_assert (line);
			
		  listing[listing_i] = g_string_new ("");			
		  //g_string_append (listing[listing_i], "");
		  listing_i++;
		}
	  line = src->listing_buffer[listing_i++];
	  g_assert (line);
	  g_assert (line->str);
		
		
	  /* if opcode */
	  if ( entry->s_op < 256 )
		{
		  IdOpcode *ido = entry->s_op_id_opcode;
		  g_assert (ido);
			
		  if ( entry->s_op > 0x0f )
			{
			  g_string_append_printf (listing[pto], "%4X %2X", entry->address, entry->s_op);
			}
		  else
			{
			  g_string_append_printf (listing[pto], "%4X 0%X", entry->address, entry->s_op);
			}
 			
			
		  /* if args */
		  if ( ido->user_args )
			{
			  if ( entry->b_opnd1 > 0x0f )
				{
				  g_string_append_printf(listing[pto], " %2X", entry->b_opnd1);
				}
			  else
				{
				  g_string_append_printf(listing[pto], " 0%X", entry->b_opnd1);
				}
			  if ( ido->user_args == 2 )
				{
				  if ( entry->b_opnd1 > 0x0f )
					{
					  g_string_append_printf(listing[pto], " %2X", entry->b_opnd2);
					}
				  else
					{
					  g_string_append_printf(listing[pto], " 0%X", entry->b_opnd2);
					}
				}
			}
		}
	  else /* if pseudo */
		{
		  if ( entry->s_op == ID_PSEUDO_DS )
			{
			  g_string_append_printf (listing[pto], "%4X <%d bytes>", entry->address, entry->b_opnd1);
			}
		  else if ( entry->s_op == ID_PSEUDO_DB )
			{
			  GString *olist;
			  gint tot = 0;
			  olist = get_hex_list (entry->s_opnd, &tot);
				
			  g_string_append_printf (listing[pto], "%4X ",entry->address);
			  g_string_append (listing[pto], olist->str);
			  g_string_append_printf(listing[pto], " (%d bytes)", tot);
				
			  g_string_free (olist, TRUE);
			}
		}
		
	  /* append with ops */
	  //g_string_append (listing[entry_i], "");
		
		
	  //g_string_append (listing[entry_i], "\n");
	}
}

gint
asm_listing_binary_array_max_len (GString **array)
{
  gint ml = 0;
  GString **line ;
	
  g_assert (array);
	
  line = array;
  while ( *line )
	{
	  gint len;
	  len = strlen((*line)->str);
	  ml = MAX(ml, len);
	  line++;
	}
	
  return ml;
}

GString *
asm_listing_generate (AsmSource * src)
{
  GString *listing = NULL;
  gint entry_i = 0, listing_i=0;
  GString *bin[ASM_SOURCE_MAX_LINES] = { NULL };
  gint i = 0;
  gint max_len = 0;
	
  g_assert (src);
  g_assert (src->entries);
	
  /* create binary first */
  asm_listing_get_binary_array (src, bin);
	
  /* get max len */
  max_len = asm_listing_binary_array_max_len (bin);
	
  listing = g_string_new (_(";Assembler generated listing. Do not hand edit and assemble\n"));
	
  while ( bin[i] )
	{
	  gint diff = 0;
	  gint orilen;
		
	  g_assert (bin[i]->str);
		
	  /* append code */
	  g_string_append (listing, bin[i]->str);
		
	  /* calculate diff */
	  orilen = strlen (bin[i]->str);
	  diff = orilen/4;
		
		
	  g_assert (diff >= 0);
		
	  g_string_append_printf (listing, "%*s ", (max_len+1)%4 - diff + 1, " ");
	  g_assert (src->listing_buffer[i]->str);
	  g_string_append ( listing, src->listing_buffer[i]->str);
	  g_string_append ( listing, "\n");
	  i++;
	}
	
  /* clean up */
  {
	GString **p = bin;
	while ( *p )
	  g_string_free (*p++, TRUE);
  }
	
  return listing;
  //debug
	
  listing = g_string_new (_(";Assembler Listing (Do Not assemble)\n\n"));
	
  for (entry_i =0; entry_i<src->entries_total; entry_i++ )
	{
	  AsmSourceEntry *entry = src->entries[entry_i];
	  gint pto;
	  GString *line;
		
	  g_assert (entry);
	  pto = entry->listing_buffer_line_no;
		
	  g_assert (pto>=listing_i);
	  /* write listing before this entry */
	  while ( pto > listing_i )
		{
		  line = src->listing_buffer[listing_i++];
		  g_assert (line);
			
		  g_string_append (listing, line->str);
		  g_string_append (listing, "\n");
		}
	  line = src->listing_buffer[listing_i++];
	  g_assert (line);
	  g_assert (line->str);
		
		
	  /* if opcode */
	  if ( entry->s_op < 256 )
		{
		  IdOpcode *ido = entry->s_op_id_opcode;
		  g_assert (ido);
			
		  g_string_append_printf (listing, "%4X %2X", entry->address, entry->s_op);
			
		  /* if args */
		  if ( ido->user_args )
			{
			  g_string_append_printf(listing, " %2X", entry->b_opnd1);
			  if ( ido->user_args == 2 )
				g_string_append_printf(listing, " %2X", entry->b_opnd2);
			}
		}
	  else /* if pseudo */
		{
		  if ( entry->s_op == ID_PSEUDO_DS )
			{
			  g_string_append_printf (listing, "%4X <%d bytes>", entry->address, entry->b_opnd1);
			}
		  else if ( entry->s_op == ID_PSEUDO_DB )
			{
			  GString *olist;
			  gint tot = 0;
			  olist = get_hex_list (entry->s_opnd, &tot);
				
			  g_string_append_printf (listing, "%4X ",entry->address);
			  g_string_append (listing, olist->str);
			  g_string_append_printf(listing, " (%d bytes)", tot);
				
			  g_string_free (olist, TRUE);
			}
		}
		
	  /* append with ops */
	  g_string_append (listing, line->str);
		
		
	  g_string_append (listing, "\n");
	}
	
	
  return listing;
}
