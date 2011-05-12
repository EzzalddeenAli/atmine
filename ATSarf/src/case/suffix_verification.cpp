#include <QTextStream>
#include <QFile>
#include "common.h"
#include <QQueue>
#include "tree.h"
#include "database_info_block.h"
#include <QString>
#include <QMainWindow>
#include <QScrollArea>
#include <QLabel>
#include <QDir>

//#define SHOW_CATEGORY

void verify(item_types type) {
	QString t=interpret_type(type);
	QQueue<node *> queue;
	node * base=(type==PREFIX?database_info.Prefix_Tree->getFirstNode():database_info.Suffix_Tree->getFirstNode());
	QFile file(QString("%1.dot").arg(t));
	file.remove();
	if (!file.open(QIODevice::ReadWrite))
	{
		out<<"Error openning file\n";
		return;
	}

	QTextStream d_out(&file);
	d_out.setCodec("utf-8");
	d_out<<"digraph tree {\n";
	queue.enqueue(base);

	while (!queue.isEmpty())
	{
		node & n=*(queue.dequeue());
		if (n.isLetterNode()) {
			letter_node & l= (letter_node &)n;
			QChar letter=l.getLetter();
			d_out<<"n"<<(long)(&n)<<" [label=\""<<(letter!='\0'?letter:'-')<<"\", shape=circle];\n";
		}else {
		#ifdef SHOW_CATEGORY
			result_node & r= (result_node &)n;
			d_out<<"n"<<(long)(&n)<<" [label=\""
				#ifdef SHOW_CATEGORY_LABEL
					<<database_info.comp_rules->getCategoryName(r.get_resulting_category_id())
				#endif
					<<"\", shape=rectangle];\n";
		#endif
		#ifdef SHOW_RAW_DATA
			int size=r.raw_datas.size();
			for (int i=0;i<size;i++) {
				d_out<<"r"<<i<<" [label=\""<<r.raw_datas[i]<<"\", shape=oval];\n";
				d_out<<"n"<<(long)(&n)<<"->"<<"r"<<i<<";\n";
			}
		#endif
		}
		QVector<letter_node *> l_nodes=n.getLetterChildren();
		QList<result_node *> r_nodes =*n.getResultChildren();
		int size=l_nodes.size();
		for (int i=0; i<size;i++) {
			node * nc=l_nodes[i];
			if (nc!=NULL) {
				queue.enqueue(nc);
				d_out<<"n"
					#ifdef SHOW_CATEGORY
						<<(long)(&n)
					#else
						<<(long)((n.isLetterNode()?&n:(n.getPrevious())))
					#endif
						<<"->"<<"n"<<(long)nc<<";\n";
			}
		}

		size=r_nodes.size();
		for (int i=0; i<size;i++) {
			node * nc=r_nodes[i];
			if (nc!=NULL) {
				queue.enqueue(nc);
			#ifdef SHOW_CATEGORY
				d_out<<"n"<<(long)(&n)<<"->"<<"n"<<(long)nc<<";\n";
			#endif
			}
		}

	}
	d_out<<"}\n";
	file.close();
	try{
		system(QString("dot -Tsvg %1.dot -o %1.svg").arg(t).toStdString().data());
#ifdef DISPLAY
		QMainWindow * mw =new QMainWindow(NULL);
		mw->setWindowTitle(QString("%1 Tree").arg(t));
		QScrollArea * sa=new QScrollArea(mw);
		mw->setCentralWidget(sa);
		QLabel *pic=new QLabel(sa);
		pic->setPixmap(QPixmap(QString("./%1.svg").arg(t)));
		sa->setWidget(pic);
		mw->show();
#else
		system(QString("kgraphviewer ./%1.dot -caption %1.dot &").arg(t).arg(QDir::currentPath()).toStdString().data());
#endif
	}
	catch(...)
	{}
}
