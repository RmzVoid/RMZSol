using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Win32.SafeHandles;

namespace RMZGui
{
	class Win32
	{
		[DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
		public static extern SafeFileHandle CreateFile(
			 [MarshalAs(UnmanagedType.LPWStr)] string fileName,
			 [MarshalAs(UnmanagedType.U4)] FileAccess desiredAccess,
			 [MarshalAs(UnmanagedType.U4)] FileShare shareMode,
			 IntPtr securityAttributes,
			 [MarshalAs(UnmanagedType.U4)] CreationDisposition creationDisposition,
			 [MarshalAs(UnmanagedType.U4)] FileAttributes flagsAndAttributes,
			 IntPtr templateFile);

		[Flags]
		public enum FileAccess : uint
		{
			GenericRead = 0x80000000,
			GenericWrite = 0x40000000,
			GenericExecute = 0x20000000,
			GenericAll = 0x10000000
		}

		[Flags]
		public enum FileShare : uint
		{
			None = 0x00000000,
			Read = 0x00000001,
			Write = 0x00000002,
			Delete = 0x00000004
		}

		public enum CreationDisposition : uint
		{
			New = 1,
			CreateAlways = 2,
			OpenExisting = 3,
			OpenAlways = 4,
			TruncateExisting = 5
		}

		[Flags]
		public enum FileAttributes : uint
		{
			Normal = 0x00000080,
			Overlapped = 0x40000000
		}

		[DllImport("kernel32.dll", SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		public static extern bool CloseHandle(IntPtr hObject);
	}
}
