
# Install flex + bison via winflexbison
if (!(Test-Path winflexbison.zip)) {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    if (Invoke-WebRequest -Uri https://github.com/lexxmark/winflexbison/releases/download/v2.5.18/win_flex_bison-2.5.18.zip -OutFile winflexbison.zip) {
        Write-Output "Could not download winflexbison"
        exit 1
    }
}

$expectedhash = "095CF65CB3F12EE5888022F93109ACBE6264E5F18F6FFCE0BDA77FEB31B65BD8"
$actualhash = (Get-FileHash -Algorithm "SHA256" winflexbison.zip).hash

if ($actualhash -ne $expectedhash) {
    Write-Output "File hash does not match: Expected: $expectedhash, Actual: $actualhash"
    exit 1
}

if (!(Test-Path winflexbison)) {
    Expand-Archive -LiteralPath winflexbison.zip -DestinationPath winflexbison
}
