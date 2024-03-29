[![Github CI](https://github.com/jafl/drakon/actions/workflows/ci.yml/badge.svg)](https://github.com/jafl/drakon/actions/workflows/ci.yml)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_drakon&metric=alert_status)](https://sonarcloud.io/dashboard?id=jafl_drakon&branch=main)

[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_drakon&metric=sqale_rating)](https://sonarcloud.io/dashboard?id=jafl_drakon&branch=main)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_drakon&metric=security_rating)](https://sonarcloud.io/dashboard?id=jafl_drakon&branch=main)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_drakon&metric=vulnerabilities)](https://sonarcloud.io/dashboard?id=jafl_drakon&branch=main)

[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_drakon&metric=ncloc)](https://sonarcloud.io/dashboard?id=jafl_drakon&branch=main)

# Drakon

Drakon provides a graphical display of the processes running on your UNIX machine.  The [on-line help](http://drakon.sourceforge.net/help.html) explains all the features of the program.


## Installing a binary

Download a package:

[![Download from Github](http://libjx.sourceforge.net/github.png)](https://github.com/jafl/drakon/releases/latest)

<a href="https://sourceforge.net/p/drakon/"><img alt="Download Drakon" src="https://sourceforge.net/sflogo.php?type=17&group_id=170488" width="250"></a>

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-white.svg)](https://snapcraft.io/nps-drakon)

If you download a tar, unpack it, cd into the directory, and run `sudo ./install`.

Otherwise, put the program (`drakon`) in a directory that is on your execution path.  `~/bin` is typically a good choice.


## Building from source

1. Install the [JX Application Framework](https://github.com/jafl/jx_application_framework),  and set the `JX_ROOT` environment variable to point to its `include` directory.
1. `./configure`
1. `make`


## FAQ

For general questions, refer to the [Programs FAQ](https://github.com/jafl/jx_application_framework/blob/master/APPS.md).
