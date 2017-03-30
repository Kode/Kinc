/************************************************************************************

Filename    :   Util_GetSystemSpecs.cpp
Content     :   This code needs to be shared by applications, but can't be in LibOVR.
                Define GET_SYSTEM_SPECS and include directly in a cpp file.
Created     :   Feb 27, 2015
Authors     :   Kevin Jenkins (moved from RiftConfigUtil)

Copyright   :   Copyright 2015 Oculus, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus Inc license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#if defined(GET_SYSTEM_SPECS)

#ifndef WCHAR_TO_OVR_STRING
//Qt redefines wchar_t , but our String has an explicit constructor.  Use this hack for desired behavior
#define WCHAR_TO_OVR_STRING(wchar_array) String() + wchar_array
#endif

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include "Util/Util_SystemInfo.h"

#if defined(OVR_OS_WIN32)

#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>
# pragma comment(lib, "wbemuuid.lib")
#include "DXGI.h"

JSON*  GetSystemSpecs()
{
    JSON* specs = JSON::CreateObject();
    HRESULT hres;

    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(hres))
    {

        return specs;                 // Program has failed.
    }

    IWbemServices *pSvc = NULL;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object 
        &pSvc                    // pointer to IWbemServices proxy
        );

    if (FAILED(hres))
    {

        pLoc->Release();
        return specs;                // Program has failed.
    }

    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
        );

    if (FAILED(hres))
    {

        pSvc->Release();
        pLoc->Release();
        return specs;               // Program has failed.
    }


    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Caption FROM Win32_OperatingSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {

        pSvc->Release();
        pLoc->Release();
        return specs;               // Program has failed.
    }

    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        // Get the value of the Name property
        hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
        specs->AddStringItem("Operating System", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
        VariantClear(&vtProp);

        pclsObj->Release();
    }
    pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Name FROM Win32_processor"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {

        pSvc->Release();
        pLoc->Release();
        return specs;               // Program has failed.
    }

    uReturn = 0;
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        // Get the value of the Name property
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        specs->AddStringItem("Processor", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Name , AdapterRam, DriverVersion, VideoModeDescription FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {

        pSvc->Release();
        pLoc->Release();
        return specs;               // Program has failed.
    }

    JSON* graphicsadapters = JSON::CreateArray();

    uReturn = 0;
    while (pEnumerator)
    {
        JSON* graphicscard = JSON::CreateObject();
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        // Get the value of the Name property
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        graphicscard->AddStringItem("Name", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
        VariantClear(&vtProp);

        // Get the value of the Name property
        hr = pclsObj->Get(L"AdapterRam", 0, &vtProp, 0, 0);
        uint32_t capacity = vtProp.uintVal;
        graphicscard->AddNumberItem("Video Controller RAM (MB)", capacity / 1048576);
        VariantClear(&vtProp);

        //get driver version
        hr = pclsObj->Get(L"DriverVersion", 0, &vtProp, 0, 0);
        graphicscard->AddStringItem("Driver Version", WCHAR_TO_OVR_STRING(vtProp.bstrVal));

        //get resolution
        hr = pclsObj->Get(L"VideoModeDescription", 0, &vtProp, 0, 0);
        graphicscard->AddStringItem("Video Mode", WCHAR_TO_OVR_STRING(vtProp.bstrVal));

        VariantClear(&vtProp);
        pclsObj->Release();

        graphicsadapters->AddArrayElement(graphicscard);
    }

    specs->AddItem("Graphics Adapters", graphicsadapters);

    pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Capacity FROM Win32_PhysicalMemory"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {

        pSvc->Release();
        pLoc->Release();
        return specs;               // Program has failed.
    }

    uint64_t totalram = 0;
    uReturn = 0;
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        // Get the value of the Name property
        hr = pclsObj->Get(L"Capacity", 0, &vtProp, 0, 0);
        uint64_t capacity = QString::fromWCharArray(vtProp.bstrVal).toLongLong();
        totalram += capacity;
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    specs->AddNumberItem("Total RAM (GB)", totalram / 1073741824.0);

    JSON* usbtree = JSON::CreateArray();

    QMap<QString, QStringList> antecedents;

    pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Antecedent,Dependent FROM Win32_USBControllerDevice"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {

        pSvc->Release();
        pLoc->Release();
        return specs;               // Program has failed.
    }

    VARIANT vtProp;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        // Get the reference value of the Antecedent property.  There is not a function to dereference the value.
        hr = pclsObj->Get(L"Antecedent", 0, &vtProp, 0, 0);
        BSTR name = vtProp.bstrVal;
        //sanitize the string input to just the output 
        QString antecedent = QString::fromWCharArray(name).split("=")[1].replace("\"", "");
        VariantClear(&vtProp);

        // Get the reference value of the Dependent property.  There is not a function to dereference the value.
        hr = pclsObj->Get(L"Dependent", 0, &vtProp, 0, 0);
        name = vtProp.bstrVal;
        //sanitize the string input to just the output 
        QString dependent = QString::fromWCharArray(name).split("=")[1].replace("\"", "");
        antecedents[antecedent].append(dependent);
        VariantClear(&vtProp);

    }
    for (int ant = 0; ant < antecedents.size(); ant++)
    {
        QString antecedent_name = antecedents.keys()[ant];
        //get antecedent object in a new enumerator
        IEnumWbemClassObject* pEnumerator2 = NULL;
        IWbemClassObject *pclsObj2;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT Manufacturer, Name, DeviceID, Caption FROM WIN32_USBController where deviceid = '") + bstr_t(antecedent_name.toUtf8()) + bstr_t("'"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator2);
        if (FAILED(hres))
        {

            pSvc->Release();
            pLoc->Release();
            return specs;               // Program has failed.
        }

        JSON* USBAntecedent = JSON::CreateObject();

        while (pEnumerator2)
        {
            HRESULT hr = pEnumerator2->Next(WBEM_INFINITE, 1,
                &pclsObj2, &uReturn);

            if (0 == uReturn)
            {
                break;
            }

            VARIANT vtProp;

            // Get the value of the Name property
            hr = pclsObj2->Get(L"Name", 0, &vtProp, 0, 0);
            USBAntecedent->AddStringItem("name", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
            VariantClear(&vtProp);

            // Get the value of the DeviceID property
            hr = pclsObj2->Get(L"DeviceID", 0, &vtProp, 0, 0);
            USBAntecedent->AddStringItem("deviceid", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
            VariantClear(&vtProp);

            // Get the value of the caption property
            hr = pclsObj2->Get(L"Caption", 0, &vtProp, 0, 0);
            USBAntecedent->AddStringItem("caption", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
            VariantClear(&vtProp);

            // Get the value of the manufacturer property
            hr = pclsObj2->Get(L"Manufacturer", 0, &vtProp, 0, 0);
            USBAntecedent->AddStringItem("manufacturer", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
            VariantClear(&vtProp);

            pclsObj2->Release();
        }
        JSON* devices = JSON::CreateArray();
        for (int dev = 0; dev < antecedents[antecedent_name].size(); ++dev)
        {
            //get antecedent object in a new enumerator
            pEnumerator2 = NULL;
            if (!pclsObj2) pclsObj2->Release();
            hres = pSvc->ExecQuery(
                bstr_t("WQL"),
                bstr_t("SELECT Manufacturer,Name FROM Win32_PnPEntity where DeviceID = '") + bstr_t(antecedents[antecedent_name][dev].toUtf8()) + bstr_t("'"),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pEnumerator2);
            if (FAILED(hres))
            {

                pSvc->Release();
                pLoc->Release();
                return specs;               // Program has failed.
            }


            while (pEnumerator2)
            {
                HRESULT hr = pEnumerator2->Next(WBEM_INFINITE, 1,
                    &pclsObj2, &uReturn);

                if (0 == uReturn)
                {
                    break;
                }

                VARIANT vtProp;

                JSON* properties = JSON::CreateObject();

                // Get the value of the Manufacturer property
                hr = pclsObj2->Get(L"Manufacturer", 0, &vtProp, 0, 0);
                properties->AddStringItem("manufacturer", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
                VariantClear(&vtProp);

                // Get the value of the Manufacturer property
                hr = pclsObj2->Get(L"Name", 0, &vtProp, 0, 0);
                properties->AddStringItem("name", WCHAR_TO_OVR_STRING(vtProp.bstrVal));
                VariantClear(&vtProp);

                pclsObj2->Release();
                devices->AddArrayElement(properties);
            }
        }

        USBAntecedent->AddItem("Devices", devices);
        usbtree->AddArrayElement(USBAntecedent);
    }

    specs->AddItem("USB Tree", usbtree);


    // Cleanup
    // ========
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    if (!pclsObj) pclsObj->Release();
    return specs;
}
#endif
#ifdef OVR_OS_MAC
JSON* GetSystemSpecs()
{
    return nullptr;
}
#endif
#ifdef OVR_OS_LINUX
JSON* GetSystemSpecs()
{
    return nullptr;
}
#endif

#endif // GET_SYSTEM_SPECS
