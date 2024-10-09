# Automatically generated by scripts/boost/generate-ports.ps1

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO boostorg/fusion
    REF boost-${VERSION}
    SHA512 ed820a5c068d1468140161f6b97c6b08e2d8543bca9fc104f14cd76845ae1103d9fcfa9be4ec1a5699da1a509f5d324b1dc06e1b0e928e1b8862dc01707438ab
    HEAD_REF master
)

set(FEATURE_OPTIONS "")
boost_configure_and_install(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS ${FEATURE_OPTIONS}
)