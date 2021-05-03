/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Ant4C.Net.Module
{
    public static class FileUnit
    {
        public static bool IsAssembly(string path, out bool is_assembly)
        {
            is_assembly = false;

            try
            {
                var assembly = Assembly.LoadFile(path);
                is_assembly = true;
            }
            catch (BadImageFormatException)
            {
            }
            catch (FileNotFoundException)
            {
                return false;
            }
            catch
            {
                return false;
            }

            return true;
        }
    }
#if NETSTANDARD2_1 || NETCOREAPP3_1
    public static class Delegates
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct FileUnit_IsAssembly_Argument
        {
            public IntPtr path;
        }

        public delegate byte FileUnit_IsAssemblyDelegate(FileUnit_IsAssembly_Argument argument);
        public static byte FileUnit_IsAssembly(FileUnit_IsAssembly_Argument argument)
        {
            bool is_assembly;
            var path_str = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
                ? Marshal.PtrToStringUni(argument.path)
                : Marshal.PtrToStringUTF8(argument.path);

            if (FileUnit.IsAssembly(path_str, out is_assembly))
            {
                return is_assembly ? (byte)1 : (byte)0;
            }

            return 2;
        }
    }
#else
    public static class Program
    {
        public static int FileNameSpace(string[] arguments)
        {
            switch (arguments[0])
            {
                case "is-assembly":
                {
                    bool is_assembly;

                    if (FileUnit.IsAssembly(arguments[1], out is_assembly))
                    {
                        return is_assembly ? 1 : 0;
                    }

                    return 2;
                }

                default:
                    break;
            }

            return -1;
        }

        public static int Main(string[] argv)
        {
            if (3 != argv.Length)
            {
                return -1;
            }

            var arguments = new string[argv.Length - 1];
            Array.Copy(argv, 1, arguments, 0, argv.Length - 1);

            switch (argv[0])
            {
                case "file":
                {
                    return FileNameSpace(arguments);
                }
            }

            return -1;
        }
    }
#endif
}
