local utils = import 'utils.libsonnet';

{
  uses_user_defaults: true,
  security_policy_supported_versions: { '0.0.x': ':white_check_mark:' },
  project_name: 'browserchooser',
  version: '0.0.2',
  description: 'Pick which browser to use based on domain names.',
  custom_project_badges: [
    {
      anchor: '[![Tests](https://github.com/Tatsh/browserchooser/actions/workflows/tests.yml/badge.svg)]',
      href: 'https://github.com/Tatsh/browserchooser/actions/workflows/tests.yml',
    },
    {
      anchor: '[![Coverage Status](https://coveralls.io/repos/github/Tatsh/browserchooser/badge.svg?branch=master)]',
      href: 'https://coveralls.io/github/Tatsh/browserchooser?branch=master',
    },
  ],
  keywords: ['browser', 'linux'],
  want_main: false,
  want_codeql: false,
  want_tests: false,
  package_json+: {
    cspell+: {
      ignorePaths+: [
        '.docs/*.tags',
        '.docs/*.tag.xml',
      ],
    },
    scripts+: {
      'flatpak-build-install': 'flatpak run --command=flathub-build org.flatpak.Builder --install sh.tat.browserchooser.yml',
      'flatpak-install': 'flatpak uninstall -y browserchooser || true && flatpak install -y --user --reinstall flathub sh.tat.browserchooser',
      'flatpak-lint': 'flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest sh.tat.browserchooser.yml',
      'flatpak-run': 'flatpak run sh.tat.browserchooser',
      'flatpak-uninstall': 'flatpak uninstall -y sh.tat.browserchooser',
    },
  },
  prettierignore+: ['*.desktop', '*.tags', '*.mm'],
  cz+: {
    commitizen+: {
      version_files+: [
        'man/browserchooser.1',
        'sh.tat.browserchooser.yml',
        'src/main.cpp',
        'snapcraft.yaml',
      ],
    },
  },
  shared_ignore+: [
    '/.flatpak-builder/',
    '/build_fp/',
    '/repo/',
  ],
  vscode+: {
    c_cpp+: {
      configurations: [
        {
          cStandard: 'c23',
          compilerPath: '/usr/bin/gcc',
          cppStandard: 'c++23',
          includePath: [
            '${workspaceFolder}/src/**',
            '${workspaceFolder}/build/src/browserchooser_autogen/include',
          ],
          name: 'Linux',
        },
      ],
    },
    settings+: {
      'files.associations': {
        '*.moc': 'cpp',
        '*.ui': 'xml',
        'i18n/*.ts': 'xml',
      },
    },
  },
  // C++ only
  cmake+: {
    uses_qt: true,
  },
  project_type: 'c++',
  vcpkg+: {
    dependencies: [
      {
        name: 'ecm',
        'version>=': utils.latestVcpkgPortVersion('ecm'),
      },
      {
        features: ['gui', 'widgets'],
        name: 'qtbase',
        'version>=': utils.latestVcpkgPortVersion('qtbase'),
      },
    ],
  },
  github+: {
    publish_winget: {
      identifier: 'Tatsh.BrowserChooser',
      max_versions_to_keep: 1,
    },
  },
  snapcraft+: {
    parts+: {
      browserchooser+: {
        source: 'https://github.com/Tatsh/browserchooser.git',
        'source-tag': 'v0.0.2',
      },
    },
  },
}
