FROM ubuntu:20.04

MAINTAINER @ebadi

ENV DEBIAN_FRONTEND="noninteractive" TZ="Europe/Sweden"

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get install -y --no-install-recommends apt-utils
RUN apt-get install -y fonts-lmodern lmodern git
RUN apt-get install -y pandoc
RUN apt-get install -y texlive-base texlive-latex-extra texlive-fonts-recommended texlive-latex-recommended texlive-xetex
RUN apt-get install -y python3-pip 
RUN apt-get install -y ttf-mscorefonts-installer texlive-fonts-extra  
RUN apt-get install -y fontconfig
RUN apt-get install -y fonts-texgyre
RUN pip3 install mkdocs
RUN apt-get install -y librsvg2-bin
