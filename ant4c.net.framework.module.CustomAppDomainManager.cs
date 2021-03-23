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
    public sealed class CustomAppDomainManager : AppDomainManager, IFrameworkNamespace
    {
        public CustomAppDomainManager()
        {
        }

        public override void InitializeNewDomain(AppDomainSetup appDomainInfo)
        {
            base.InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;
        }

        [return: MarshalAs(UnmanagedType.LPWStr)]
        public string GetRuntimeFramework()
        {
            return string.Format("{0}", VersionDetector.GetVersion());
        }

        [return: MarshalAs(UnmanagedType.I1)]
        public bool Exists([MarshalAs(UnmanagedType.LPWStr)] string framework)
        {
            return VersionDetector.Exists(framework);
        }
    }
}
