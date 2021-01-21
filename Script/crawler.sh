

RepoDir=$1
Repositories=`ls $RepoDir`
for file in $Repositories; 
do
	Repo="Repository/"$file
   	./crawler.py -f sniffer -r $Repo
done



