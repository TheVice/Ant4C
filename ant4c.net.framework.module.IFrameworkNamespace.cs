/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

using System;
using System.Runtime.InteropServices;

namespace Ant4C.Net.Framework.Module
{
    [ComImport, Guid("49467261-6D65-776F-726B-4E616D657370"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IFrameworkNamespace
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetRuntimeFramework();

        [return: MarshalAs(UnmanagedType.I1)]
        bool Exists([MarshalAs(UnmanagedType.LPWStr)] string framework);
    }
}
