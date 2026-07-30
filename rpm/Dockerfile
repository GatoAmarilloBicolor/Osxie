ARG OS=fedora:37
FROM ${OS}

SHELL ["bash", "-euxvc"]

RUN dnf install -y rpm-build dnf-utils rpmdevtools git; \
    source /etc/os-release; \
    dnf install -y https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-${VERSION_ID}.noarch.rpm; \
    dnf clean all

COPY SPECS/osxie.spec /osxie.spec

# Bootstrap
RUN yum-builddep -y /osxie.spec; \
    dnf clean all

RUN rm /osxie.spec

RUN mkdir -p /root/rpmbuild/SOURCES

CMD bash -xv /src/rpm/build.bsh
