
# Install flex + bison via winflexbison
if (!(Test-Path winflexbison.zip)) {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    if (Invoke-WebRequest -Uri https://github.com/lexxmark/winflexbison/releases/download/v2.5.23/win_flex_bison-2.5.23.zip -OutFile winflexbison.zip) {
        Write-Output "Could not download winflexbison"
        exit 1
    }
}

$expectedhash = "6AA5C8EA662DA1550020A5804C28BE63FFAA53486DA9F6842E24C379EC422DFC"
$actualhash = (Get-FileHash -Algorithm "SHA256" winflexbison.zip).hash

if ($actualhash -ne $expectedhash) {
    Write-Output "File hash does not match: Expected: $expectedhash, Actual: $actualhash"
    exit 1
}

if (!(Test-Path winflexbison)) {
    Expand-Archive -LiteralPath winflexbison.zip -DestinationPath winflexbison
}
