/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

using System;
using System.IO;
using System.Reflection;

namespace Ant4C.Net.Framework.Module
{
    public static class VersionDetector
    {
        private static Version VersionParse(string input, Version version)
        {
            var digits = "0123456789".ToCharArray();

            if (string.IsNullOrEmpty(input))
            {
                return version;
            }

            var index = input.IndexOfAny(digits);

            if (-1 == index)
            {
                return version;
            }

            input = input.Substring(index);
            index = input.LastIndexOfAny(digits);

            if (index + 1 < input.Length)
            {
                input = input.Substring(0, index + 1);
            }

            return new Version(input);
        }

        private static bool IsTypeExists(string type_name)
        {
            if (string.IsNullOrEmpty(type_name))
            {
                return false;
            }

            var assembly = Assembly.GetAssembly(typeof(object));

            foreach (Type type in assembly.GetTypes())
            {
                if (type_name == type.FullName.ToLower())
                {
                    return true;
                }
            }

            return false;
        }

        private static Version GetVersionAt4dot7dot1AndAbove(Version version)
        {
            var version_in_the_string = string.Empty;
            /*var version_in_the_string = System.Runtime.InteropServices.RuntimeInformation.FrameworkDescription;*/

            try
            {
                var the_type = Type.GetType("System.Runtime.InteropServices.RuntimeInformation");

                if (the_type != null)
                {
                    var method_name = "get_FrameworkDescription".ToLower();

                    foreach (var the_method in the_type.GetMethods())
                    {
                        if (method_name == the_method.Name.ToLower())
                        {
                            version_in_the_string = the_method.Invoke(the_type, null) as string;
                            break;
                        }
                    }
                }
            }
            catch
            {
            }

            try
            {
                version = VersionParse(version_in_the_string, version);
            }
            catch
            {
            }

            return version;
        }

        public static Version GetVersion()
        {
            var version = Environment.Version;

            if (IsTypeExists("System.Runtime.InteropServices.RuntimeInformation".ToLower()))
            {
                return GetVersionAt4dot7dot1AndAbove(version);
            }
            else if (IsTypeExists("System.ValueTuple".ToLower()))
            {
                return new Version("4.7");
            }
            else if (IsTypeExists("System.Diagnostics.Tracing.EventSourceCreatedEventArgs".ToLower()))
            {
                return new Version("4.6.2");
            }
            else if (IsTypeExists("System.AppContext".ToLower()))
            {
                try
                {
                    var assembly = Assembly.GetAssembly(typeof(object));
                    var path = Path.GetDirectoryName(assembly.Location);
                    path = Path.Combine(path, "System.ComponentModel.DataAnnotations.dll");

                    if (File.Exists(path))
                    {
                        assembly = Assembly.LoadFrom(path);
                        var the_type = assembly.GetType("System.ComponentModel.DataAnnotations.RegularExpressionAttribute");

                        if (the_type != null)
                        {
                            var property_name = "MatchTimeoutInMilliseconds".ToLower();

                            foreach (var the_property in the_type.GetProperties())
                            {
                                if (property_name == the_property.Name.ToLower())
                                {
                                    return new Version("4.6.1");
                                }
                            }
                        }
                    }
                }
                catch
                {
                }

                return new Version("4.6");
            }
            else if (IsTypeExists("System.Collections.ObjectModel.ReadOnlyDictionary`2".ToLower()))//TypeInfo
            {
                try
                {
                    var assembly = Assembly.GetAssembly(typeof(object));
                    var path = Path.GetDirectoryName(assembly.Location);
                    path = Path.Combine(path, "System.Transactions.dll");

                    if (File.Exists(path))
                    {
                        assembly = Assembly.LoadFrom(path);
                        var the_type = assembly.GetType("System.Transactions.Transaction");

                        if (the_type != null)
                        {
                            var method_name = "PromoteAndEnlistDurable".ToLower();

                            foreach (var the_method in the_type.GetMethods())
                            {
                                if (method_name == the_method.Name.ToLower())
                                {
                                    return new Version("4.5.2");
                                }
                            }
                        }
                    }
                }
                catch
                {
                }

                try
                {
                    var the_type = Type.GetType("System.Diagnostics.Tracing.EventSource");

                    if (the_type != null)
                    {
                        var method_name = "WriteEventWithRelatedActivityId".ToLower();

                        foreach (var the_method in the_type.GetMethods())
                        {
                            if (method_name == the_method.Name.ToLower())
                            {
                                return new Version("4.5.1");
                            }
                        }
                    }
                }
                catch
                {
                }

                return new Version("4.5");
            }
            else if (new Version("4.0") < version)
            {
                return version;
            }
            else if (IsTypeExists("System.Action`1".ToLower()))
            {
                try
                {
                    var assembly = Assembly.GetAssembly(typeof(object));
                    var path = Path.GetDirectoryName(assembly.Location);
                    path = Path.GetDirectoryName(path);
                    path = Path.Combine(path, Path.Combine("v3.5", "Microsoft.Build.Tasks.v3.5.dll"));

                    if (File.Exists(path))
                    {
                        assembly = Assembly.LoadFrom(path);
                        var the_type = assembly.GetType("Microsoft.Build.Tasks.AL");

                        if (the_type != null)
                        {
                            return new Version("3.5");
                        }
                    }
                }
                catch
                {
                }

                try
                {
                    var assembly = Assembly.GetAssembly(typeof(object));
                    var path = Path.GetDirectoryName(assembly.Location);
                    path = Path.GetDirectoryName(path);
                    path = Path.Combine(path, Path.Combine("v3.0", Path.Combine("WPF", "PresentationUI.dll")));

                    if (File.Exists(path))
                    {
                        assembly = Assembly.LoadFrom(path);
                        var the_type = assembly.GetType("System.Windows.Documents.PresentationUIStyleResources");

                        if (the_type != null)
                        {
                            return new Version("3.0");
                        }
                    }
                }
                catch
                {
                }

                return new Version("2.0");
            }

            return version;
        }

        public static bool Exists(string framework)
        {
            var empty = new Version();
            var required_vesion = VersionParse(framework, empty);

            if (empty == required_vesion ||
                required_vesion < new Version(Environment.Version.Major, Environment.Version.Minor))
            {
                return false;
            }

            return required_vesion <= GetVersion();
        }
    }
}
