<#
Copyright 2019 NVIDIA CORPORATION

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
#>

param(
[Parameter(Mandatory=$true)][string]$source=$null,
[string]$output="out.exe"
)
$filename = $output

$triesLeft = 4
$delay = 2
do
{
    $triesLeft -= 1

    try
    {
        Write-Host "Downloading from bootstrap.packman.nvidia.com ..."
        $wc = New-Object net.webclient
        $wc.Downloadfile($source, $fileName)
        exit 0
    }
    catch
    {
        Write-Host "Error downloading $source!"
        Write-Host $_.Exception|format-list -force
        if ($triesLeft)
        {
            Write-Host "Retrying in $delay seconds ..."
            Start-Sleep -seconds $delay
        }
        $delay = $delay * $delay
    }
} while ($triesLeft -gt 0)
# We only get here if the retries have been exhausted, remove any left-overs:
if (Test-Path $fileName)
{
    Remove-Item $fileName
}
exit 1