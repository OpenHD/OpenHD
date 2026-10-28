# WifiTest.exe signing

The integrated GUI workflow builds `WifiTest.exe`, embeds the OpenHD logo,
tests the package, and uploads `openhd-gui-windows`. Builds are unsigned by default. A manual run can explicitly select
`test-signing` or `release-signing` to enable the separate signing job.
Fork/PR builds publish only unsigned artifacts. The signing job depends on Windows, not Pi.

The existing OpenHD SignPath organization/project settings are used:

- Organization: `630b3214-208a-4670-bfa3-1955f878b88c`
- Project: `OpenHD`, with GitHub origin `OpenHD/OpenHD`
- Artifact configuration slug: `wifi-test`; install the XML from
  [wifi-test.xml](../.github/signpath/wifi-test.xml) in that SignPath project.
- GitHub Actions secret: `SIGNPATH_API_TOKEN`, with access to this project.
- For the optional `test-signing` policy, GitHub Actions variable:
  `SIGNPATH_TEST_CERTIFICATE_BASE64`, containing the Base64 DER public test
  certificate. Never put the private key or API token in source control.

The workflow validates the effective secrets/variables, including inherited
organization settings. Missing settings fail visibly rather than claiming
the unsigned package was signed. SignPath must also have the matching policy
and GitHub origin verification configured in its account.

The default policy is `unsigned`, so absent signing configuration never blocks
a normal build. A manual run can select `test-signing`, or `release-signing` once that policy has a valid public certificate and
the necessary approvals. The job verifies a valid, timestamped Authenticode
signature before publishing a signed package. Test signatures additionally
must match the configured certificate thumbprint; trust is installed only in
the ephemeral GitHub-hosted signing runner, never on the user's PC.

Artifacts are `openhd-gui-windows-Test-Signed` for test signing and
`openhd-gui-windows-Signed` for release signing. Test certificates are not
publicly trusted and do not remove Windows trust prompts on other machines.

See SignPath's [GitHub integration](https://docs.signpath.io/trusted-build-systems/github).
