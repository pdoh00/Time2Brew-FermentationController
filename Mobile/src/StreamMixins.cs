using System;
using System.IO;

namespace FermentationController
{
	public static class StreamMixins
	{
		public static UInt16 ReadUInt16(this Stream s)
		{
			var buffer = new byte[2];
			s.Read(buffer, 0, 2);
			return BitConverter.ToUInt16 (buffer, 0);
		}

		public static UInt32 ReadUInt32(this Stream s)
		{
			var buffer = new byte[4];
			s.Read(buffer, 0, 4);
			return BitConverter.ToUInt32 (buffer, 0);
		}

		public static string ReadString(this Stream s, int length)
		{
			var buffer = new byte[length];
			s.Read (buffer, 0, length);
			return System.Text.Encoding.UTF8.GetString(buffer, 0, length);
		}
	}
}


