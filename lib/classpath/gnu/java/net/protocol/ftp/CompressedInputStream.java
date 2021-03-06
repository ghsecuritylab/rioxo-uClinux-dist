/* CompressedInputStream.java --
   Copyright (C) 2003, 2004  Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.
 
GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


package gnu.java.net.protocol.ftp;

import java.io.IOException;
import java.io.InputStream;
import java.net.ProtocolException;

/**
 * A DTP input stream that implements the FTP compressed transfer mode.
 *
 * @author Chris Burdess (dog@gnu.org)
 */
class CompressedInputStream
  extends DTPInputStream
{

  static final int EOF = 64;

  static final int RAW = 0x00;
  static final int COMPRESSED = 0x80;
  static final int FILLER = 0xc0;

  int descriptor;
  int max = -1;
  int count = -1;

  int state = RAW;              // RAW | STATE | FILLER
  int rep;                      // the compressed byte
  int n = 0;                    // the number of compressed or raw bytes

  CompressedInputStream(DTP dtp, InputStream in)
  {
    super(dtp, in);
  }

  public int read()
    throws IOException
  {
    if (transferComplete)
      {
        return -1;
      }
    if (count == -1)
      {
        readHeader();
      }
    if (max < 1)
      {
        close();
        return -1;
      }
    if (n > 0 && (state == COMPRESSED || state == FILLER))
      {
        n--;
        return rep;
      }
    int c = in.read();
    if (c == -1)
      {
        close();
      }
    count++;
    if (count >= max)
      {
        count = -1;
        if (descriptor == EOF)
          {
            close();
          }
      }
    if (c == -1)
      {
        return c;
      }
    while (n == 0)              // read code header
      {
        state = (c & 0xc0);
        n = (c & 0x3f);
        c = in.read();
        if (c == -1)
          {
            return -1;
          }
      }
    switch (state)
      {
      case RAW:
        break;
      case COMPRESSED:
      case FILLER:
        rep = c;
        break;
      default:
        throw new ProtocolException("Illegal state: " + state);
      }
    n--;
    return c;
  }
  
  public int read(byte[] buf)
    throws IOException
  {
    return read(buf, 0, buf.length);
  }
  
  public int read(byte[] buf, int off, int len)
    throws IOException
  {
    if (transferComplete)
      {
        return -1;
      }
    if (count == -1)
      {
        readHeader();
      }
    if (max < 1)
      {
        close();
        return -1;
      }
    // TODO improve performance
    for (int i = off; i < len; i++)
      {
        int c = read();
        if (c == -1)
          {
            close();
            return i;
          }
        buf[i] = (byte) c;
      }
    return len;
    /*
       int l = in.read (buf, off, len);
       if (l==-1)
       {
       close ();
       }
       count += l;
       if (count>=max)
       {
       count = -1;
       if (descriptor==EOF)
       {
       close ();
       }
       }
       return l;
     */
  }
  
  /**
   * Reads the block header.
   */
  void readHeader()
    throws IOException
  {
    descriptor = in.read();
    int max_hi = in.read();
    int max_lo = in.read();
    max = (max_hi << 8) | max_lo;
    count = 0;
  }

  /**
   * Reads the code header.
   */
  void readCodeHeader()
    throws IOException
  {
    int code = in.read();
    state = (code & 0xc0);
    n = (code & 0x3f);
  }

}

