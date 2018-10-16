./mvnw clean install;
ssh avinesh@sherlock.ukp.informatik.tu-darmstadt.de 'systemctl --user stop sherlock';
rsync -ruz ./dist/ukpsummarizer-dist-bin.tar avinesh@sherlock.ukp.informatik.tu-darmstadt.de:/srv/sherlock/dist;
ssh avinesh@sherlock.ukp.informatik.tu-darmstadt.de 'rm -rf /srv/sherlock/bin/*';
ssh avinesh@sherlock.ukp.informatik.tu-darmstadt.de 'tar xf /srv/sherlock/dist/ukpsummarizer-dist-bin.tar -C /srv/sherlock/bin';
ssh avinesh@sherlock.ukp.informatik.tu-darmstadt.de 'systemctl --user start sherlock';

