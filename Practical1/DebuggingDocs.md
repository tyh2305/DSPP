# Debugging Documentation in the process of setting up project

# Project Setup

> **Error #01**: Unable to run two `main` function in one project
>
> Solution: Create two projects, one for server, one for client

>**Error #02**: Weird symbol `╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠╠` appear on client side
> 
> Remark: JetBrains Rider Issue, not occur on VS2022
> 
> Steps: 
> 1. ╠ is 0xCC in codepage 437, and MSVC fills 0xCC to uninitialized memory to help debugging.
> 2. 0xCC appear when the code is compiled with the /GZ option,
     uninitialized variables are automatically assigned
     to this value (at byte level).
> 3. /GZ is deprecated since Visual Studio 2005; use /RTC (Run-Time Error Checks) instead.
> 4. Modify one or both of the following properties: Basic Runtime Checks or Smaller Type Check. (`Configuration Properties > C/C++ > Code Generation`)
> 5. Changed `Basic Runtime Checks` from `Both` to `Default`

> **Error #03**: Client app only runs once and `Process finished with exit code -1,073,741,819.
> 
>  Remarks: Application Error code 0xc0000005, also known as Access Violation error
>  Solution: Comment out `line 58`, as the variable is not used in other place.

> **Error #04**: Server on works on first attempt
> 
> Remarks: Error not handled properly
> The default while loop did not include `WSAStartup`, after closing the socket, it is not able to receive data properly
> Solution: Add `WSAStartup` to the while loop