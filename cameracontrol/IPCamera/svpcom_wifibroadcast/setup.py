#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
from setuptools import setup, find_packages, command
import distutils.command.bdist_rpm as orig
import setuptools.command.bdist_rpm as orig2

class bdist_rpm(orig.bdist_rpm):
    """
    Override the default bdist_rpm behavior to do the following:

    1. Run egg_info to ensure the name and version are properly calculated.
    2. Always run 'install' using --single-version-externally-managed to
       disable eggs in RPM distributions.
    3. Replace dash with underscore in the version numbers for better RPM
       compatibility.
    4. Add %global debug_package %{nil} to spec file
    """

    def run(self):
        # ensure distro name is up-to-date
        self.run_command('egg_info')

        orig.bdist_rpm.run(self)

    def _make_spec_file(self):
        version = self.distribution.get_version()
        rpmversion = version.replace('-', '_')
        spec = orig.bdist_rpm._make_spec_file(self)
        line23 = '%define version ' + version
        line24 = '%define version ' + rpmversion
        spec = ['%global debug_package %{nil}'] + \
               [
                   line.replace(
                       "Source0: %{name}-%{version}.tar",
                       "Source0: %{name}-%{unmangled_version}.tar"
                ).replace(
                    "setup.py install ",
                    "setup.py install --single-version-externally-managed "
                ).replace(
                    "%setup",
                    "%setup -n %{name}-%{unmangled_version}"
                ).replace(line23, line24)
                   for line in spec
               ]
        insert_loc = spec.index(line24) + 1
        unmangled_version = "%define unmangled_version " + version
        spec.insert(insert_loc, unmangled_version)
        return spec

orig2.bdist_rpm = bdist_rpm

version = os.environ.get('VERSION') or 'trunk'
commit = os.environ.get('COMMIT')

if version and commit:
    with open('telemetry/conf/site.cfg', 'w') as fd:
        fd.write("# Don't make any changes here, use local.cfg instead!\n\n[common]\nversion = %r\ncommit = %r\n" % (version, commit))

setup(
    url="http://github.com/svpcom/wifibroadcast",
    name="wifibroadcast",
    version=version,
    packages=find_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"]),
    zip_safe=False,
    entry_points={'console_scripts': ['wfb-cli=telemetry.cli:main']},
    scripts=['scripts/wfb-server.sh'],
    package_data={'telemetry.conf': ['master.cfg', 'site.cfg']},
    data_files = [('/usr/bin', ['wfb_tx', 'wfb_rx', 'wfb_keygen']),
                  ('/lib/systemd/system', ['scripts/wifibroadcast.service',
                                           'scripts/wifibroadcast@.service']),
                  ('/etc/default', ['scripts/default/wifibroadcast'])],

    keywords="wifibroadcast",
    author="Vasily Evseenko",
    author_email="svpcom@p2ptech.org",
    description="Wifibroadcast server",
    license="GPLv3",
)
