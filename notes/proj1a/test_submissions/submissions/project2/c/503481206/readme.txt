503481206
Gene Auyeung
003471455
Richard Ho

For reference:
$ diff BTreeNode.cc BTreeNode.cc.20 | sed -e "/^[^<]/d" | wc -l
53

$ diff BTreeNode.cc BTreeNode.cc.20 | sed -e "/^[^>]/d" | wc -l
43

$ wc -l BTreeNode.cc.20 
460 BTreeNode.cc.20

Thanks!
