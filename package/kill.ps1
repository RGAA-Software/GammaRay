Get-WmiObject Win32_Process -Filter "name = 'GammaRay.exe'" | Invoke-WmiMethod -Name Terminate | Out-Null
Get-WmiObject Win32_Process -Filter "name = 'GammaRayClientInner.exe'" | Invoke-WmiMethod -Name Terminate | Out-Null
Get-WmiObject Win32_Process -Filter "name = 'GammaRayGuard.exe'" | Invoke-WmiMethod -Name Terminate | Out-Null
Get-WmiObject Win32_Process -Filter "name = 'GammaRayRender.exe'" | Invoke-WmiMethod -Name Terminate | Out-Null
Get-WmiObject Win32_Process -Filter "name = 'GammaRayService.exe'" | Invoke-WmiMethod -Name Terminate | Out-Null
Get-WmiObject Win32_Process -Filter "name = 'GammaRayServiceManager.exe'" | Invoke-WmiMethod -Name Terminate | Out-Null