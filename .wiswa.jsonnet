{
  security_policy_supported_versions: { '0.0.x': ':white_check_mark:' },
  project_name: 'browserchooser',
  version: '0.0.1',
  description: 'Pick which browser to use based on domain names.',
  keywords: ['browser', 'linux'],
  want_main: false,
  want_codeql: false,
  want_tests: false,
  copilot+: {
    intro: 'Browser Chooser is a small application that allows you to choose which browser to open based on the domain name of the link you clicked. It is useful for people who want to use different browsers for different purposes, such as work and personal browsing.',
  },
  package_json+: {
    cspell+: {
      ignorePaths+: [
        '.docs/*.tags',
        '.docs/*.tag.xml',
      ],
    },
    scripts+: {
      'check-formatting': "clang-format -n src/*.cpp src/*.h && prettier -c . && markdownlint-cli2 '**/*.md' '#node_modules' '#vcpkg_installed'",
      'flatpak-build-install': 'flatpak run --command=flathub-build org.flatpak.Builder --install sh.tat.browserchooser.yml',
      'flatpak-install': 'flatpak uninstall -y browserchooser || true && flatpak install -y --user --reinstall flathub sh.tat.browserchooser',
      'flatpak-lint': 'flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest sh.tat.browserchooser.yml',
      'flatpak-run': 'flatpak run sh.tat.browserchooser',
      'flatpak-uninstall': 'flatpak uninstall -y sh.tat.browserchooser',
      format: 'clang-format -i src/*.cpp src/*.h && yarn prettier -w .',
    },
  },
  prettierignore+: ['*.desktop', '*.tags', '*.m*'],
  cz+: {
    commitizen+: {
      version_files+: [
        'man/browserchooser.1',
        'sh.tat.browserchooser.yml',
        'src/main.cpp',
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
        'version>=': '6.7.0',
      },
      {
        features: ['gui', 'widgets'],
        name: 'qtbase',
        'version>=': '6.8.3',
      },
    ],
  },
  github+: {
    publish_winget: {
      identifier: 'Tatsh.BrowserChooser',
      max_versions_to_keep: 1,
    },
  },
}
