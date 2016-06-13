if [[ "$TRAVIS_TAG" == v* ]]; then
  export CHANNEL="main"
else
  export CHANNEL="dev"
fi

echo "Uploading to $CHANNEL"
echo anaconda -t $ANACONDA_TOKEN upload --force --user csdms --channel $CHANNEL $HOME/miniconda/conda-bld/**/sedflux*bz2
anaconda -t $ANACONDA_TOKEN upload --force --user csdms --channel $CHANNEL $HOME/miniconda/conda-bld/**/sedflux*bz2

echo "Done."
