using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RMZGui
{
	static class Program
	{
		//
		// To allow only one instance of this app
		static Mutex mutex = new Mutex(true, "fe2a2d27-11bc-4082-b794-9e6c247caf46");

		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			if (mutex.WaitOne(TimeSpan.Zero, true))
			{
				Application.EnableVisualStyles();
				Application.SetCompatibleTextRenderingDefault(false);
				Application.Run(new MainForm());
			}
			else
			{
				MessageBox.Show("Application already running", "RmzGui", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			}
		}
	}
}
