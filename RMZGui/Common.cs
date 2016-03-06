using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RMZGui
{
	public class Util
	{
		public static string ToHex(byte[] bytes, int offset, int count)
		{
			if ((offset + count) > bytes.Length)
				return "overflow";

			char[] c = new char[count * 2];

			byte b;

			for (int bx = offset, cx = 0; bx < offset + count; ++bx, ++cx)
			{
				b = ((byte)(bytes[bx] >> 4));
				c[cx] = (char)(b > 9 ? b + 0x37 + 0x20 : b + 0x30);

				b = ((byte)(bytes[bx] & 0x0F));
				c[++cx] = (char)(b > 9 ? b + 0x37 + 0x20 : b + 0x30);
			}

			return new string(c);
		}
	}
}
