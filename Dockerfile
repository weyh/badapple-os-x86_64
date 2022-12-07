FROM bjrowlett2/x86_64-elf-gcc

RUN apt-get update
RUN apt-get install -y nasm xorriso grub-pc-bin grub-common
RUN apt-get install -y ffmpeg wget tar
RUN apt-get install -y python3 python3-pip
RUN pip3 install yt-dlp

RUN wget https://github.com/TheZoraiz/ascii-image-converter/releases/download/v1.12.0/ascii-image-converter_Linux_amd64_64bit.tar.gz
RUN tar -xvf ascii-image-converter_Linux_amd64_64bit.tar.gz
RUN mv ascii-image-converter_Linux_amd64_64bit/ascii-image-converter /bin

RUN rm -rf ascii-image-converter_Linux_amd64_64bit
RUN rm -f ascii-image-converter_Linux_amd64_64bit.tar.gz

VOLUME /root/env
WORKDIR /root/env
