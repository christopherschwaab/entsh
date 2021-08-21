using Microsoft.Win32.SafeHandles;
using System.IO;
using System.Runtime.InteropServices;
using System;


namespace consoleapp
{
    class Program
    {
        [DllImport("kernel32.dll", CharSet=CharSet.Auto, CallingConvention=CallingConvention.StdCall, SetLastError=true)]
        private static extern IntPtr CreateFile(
            string lpFileName,
            uint dwDesiredAccess,
            uint dwShareMode,
            IntPtr SecurityAttributes,
            uint dwCreationDisposition,
            uint dwFlagsAndAttributes,
            IntPtr hTemplateFile);

        [DllImport("kernel32.dll", SetLastError=true)]
        private static extern bool SetStdHandle(int nStdHandle, IntPtr hHandle);

        // [DllImport("kernel32.dll", SetLastError=true)]
        // private static extern IntPtr GetStdHandle(int nStdHandle);

        private const uint GENERIC_READ = 0x80000000;
        private const uint GENERIC_WRITE = 0x40000000;
        private const uint FILE_SHARE_READ = 0x00000001;
        private const uint FILE_SHARE_WRITE = 0x00000002;
        private const uint FILE_ATTRIBUTE_NORMAL = 0x80;
        private const uint OPEN_EXISTING = 3;

        private const int STD_OUTPUT_HANDLE = -11;

        static void Main(string[] args)
        {
            Console.Write("Write directly to CONOUT$ [Y/n]: ");
            var line = Console.ReadLine().Trim().ToLower();
            if (line == "n") {
                Console.WriteLine("Exiting without doing anything.");
                return;
            }

            // var oldStdout = GetStdHandle(STD_OUTPUT_HANDLE);
            TextWriter oldOut = Console.Out;

            Console.WriteLine("About to write 'hello\\n' to CONOUT$ (should only be visible from cmd).");

            IntPtr stdoutHandle = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, IntPtr.Zero, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, IntPtr.Zero);
            var safeHandle = new SafeFileHandle(stdoutHandle, true);
            SetStdHandle(STD_OUTPUT_HANDLE, stdoutHandle);
            var fs = new FileStream(safeHandle, FileAccess.Write);
            var writer = new StreamWriter(fs) { AutoFlush = true };
            Console.SetOut(writer);
            Console.WriteLine("hello");

            Console.SetOut(oldOut);
            Console.WriteLine("Swapped back to original output handle.");

            return;
        }
    }
}
