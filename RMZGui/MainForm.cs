using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Win32.SafeHandles;

namespace RMZGui
{
	public partial class MainForm : Form
	{
		private FileStream readStream;
		private SafeFileHandle deviceHandle;

		public MainForm()
		{
			InitializeComponent();

			//
			// Open device
			//
			try
			{
				deviceHandle = Win32.CreateFile(@"\\.\rmzdrv",
					Win32.FileAccess.GenericRead,
					Win32.FileShare.Read,
					IntPtr.Zero,
					Win32.CreationDisposition.OpenExisting,
					Win32.FileAttributes.Normal,
					IntPtr.Zero);

				if (deviceHandle.IsInvalid)
					Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());

				readStream = new FileStream(deviceHandle, FileAccess.Read, 4096, false);
			}
			catch (Exception exception)
			{
				MessageBox.Show(exception.Message, exception.Source, MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		private void btnRead_Click(object sender, EventArgs e)
		{
			if(readStream.CanRead)
			{
				byte[] buffer = new byte[4096];
				int bytesReaded = readStream.Read(buffer, 0, 4096);

				if(bytesReaded>0)
				{
					string readedString = Encoding.Unicode.GetString(buffer, 0, bytesReaded);

					tbDeviceName.Text = readedString;
				}
				else
				{
					tbDeviceName.Text = "No data";
				}
			}
		}

		private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
		{
		}
	}
}
