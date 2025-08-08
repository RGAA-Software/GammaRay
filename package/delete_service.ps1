$serviceName = "GammaRayService"

# 检查服务是否存在
$service = Get-Service -Name $serviceName -ErrorAction SilentlyContinue

if ($service) {
    # 停止服务（如果运行中）
    if ($service.Status -eq "Running") {
        Write-Host "正在停止服务: $serviceName ..."
        Stop-Service -Name $serviceName -Force
        Write-Host "服务已停止。"
    }

    # 删除服务
    Write-Host "正在删除服务: $serviceName ..."
    sc.exe delete $serviceName
    Write-Host "服务已成功删除。"
} else {
    Write-Host "服务 $serviceName 不存在。"
}