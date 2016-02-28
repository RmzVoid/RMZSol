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
		// size of buffer for read and write
		private const int bufferSize = 16384;

		// stream for data exchange with device
		private FileStream deviceStream = null;

		// deice handle
		private SafeFileHandle deviceHandle = null;

		public MainForm()
		{
			InitializeComponent();

			try
			{
				//
				// Open device
				deviceHandle = Win32.CreateFile(@"\\.\rmzdrv",
					Win32.FileAccess.GenericRead | Win32.FileAccess.GenericWrite,
					Win32.FileShare.None,
					IntPtr.Zero,
					Win32.CreationDisposition.OpenExisting,
					Win32.FileAttributes.Normal,
					IntPtr.Zero);

				if (deviceHandle.IsInvalid)
					Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());

				//
				// Open stream for data flow
				deviceStream = new FileStream(deviceHandle, FileAccess.ReadWrite, bufferSize, false);

				if (deviceStream.CanRead) btnRead.Enabled = true;
				if (deviceStream.CanWrite) btnWrite.Enabled = true;
			}
			catch (Exception exception)
			{
				MessageBox.Show(exception.Message, exception.Source, MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		private void btnRead_Click(object sender, EventArgs e)
		{
			if(deviceStream.CanRead)
			{
				byte[] buffer = new byte[4096];

				int bytesReaded = deviceStream.Read(buffer, 0, 4096);

				if(bytesReaded>0)
				{
					ulong value = buffer[0];

					//string readedString = Encoding.GetEncoding(1251).GetString(buffer);
					//string readedString = Encoding.Unicode.GetString(buffer, 0, bytesReaded);

					tbData.Text = value.ToString();//readedString;
				}
				else
				{
					tbData.Text = "No data";
				}
			}
		}

		private void btnWrite_Click(object sender, EventArgs e)
		{
			if (tbData.Text.Length <= 0)
				return;

			if (deviceStream.CanWrite)
			{
				byte[] buffer = Encoding.Unicode.GetBytes(tbData.Text); ;

				if (buffer.Length > 0)
				{
					try
					{
						deviceStream.Write(buffer, 0, buffer.Length);
						deviceStream.Flush();
					}
					catch (Exception exception)
					{
						MessageBox.Show(exception.Message, exception.Source, MessageBoxButtons.OK, MessageBoxIcon.Error);
					}
				}
				else
				{
					tbData.Text = "Buffer with zero length";
				}
			}
			else
			{
				tbData.Text = "Device not writable";
			}
		}

		private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
		{
			// here I decide to CloseHandle of devceHandle
			// but SafeFileHandle closes by self
			// so here just empty space

			if (deviceStream != null)
				deviceStream.Close();
		}
	}
}
