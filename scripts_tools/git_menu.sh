#echo "Version simple"
#git for-each-ref --sort=creatordate --format '%(creatordate:short) %(refname:short)' refs/tags

#Ver solo ultimo tag 
echo "ultimo tag "

git for-each-ref --sort=-creatordate --format '%(creatordate:format:%Y-%m-%d %H:%M) %(refname:short)' refs/tags | head -1


echo "Orden Inverso"

git for-each-ref --sort=-creatordate --format '%(creatordate:format:%Y-%m-%d %H:%M) %(refname:short)' refs/tags


echo "Author"
git for-each-ref --sort=creatordate --format '%(creatordate:format:%Y-%m-%d %H:%M) %(refname:short) - %(taggername)' refs/tags
