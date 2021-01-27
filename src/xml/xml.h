#ifndef __XML_H
#define __XML_H


#include <string>
#include <vector>

#include "priv/expat.h"

using std::string;
using std::vector;
using std::pair;

namespace SCADA
{

//*************************************************
//* XMLNode                                       *
//*************************************************
class XMLNode
{
    public:
	//Data
	enum SaveView
	{
        BrOpenPrev		= 0x01,		//Break before open tag 在某个标签(\n< \n<?)前插入\n
        BrOpenPast		= 0x02,		//Break after open tag 在某个标签(>\n  />\n)
        BrClosePast		= 0x04,		//Break after close tag 在结束标志(只有 >\n />\n ?>\n)后加入 \n
        BrTextPast		= 0x08,		//Break after text 在文本后插入 \n
        BrPrcInstrPast	= 0x10,		//Break after process instruction 在处理指令之后插入 \n
        BrAllPast		= 0x1E,		//Break after all 所有元素后插入 \n
        XMLHeader		= 0x20,		//Include XML header <?xml version='1.0' encoding='UTF-8' ?> 在版本头后插入 \n
        MissTagEnc		= 0x100,	//Miss tag name encode 名称是否要编码
        MissAttrEnc		= 0x200		//Miss attribute name encode 属性是否要编码
	};

	//Methods
	XMLNode( const string &name = "" ) : mName(name), mText(""), mParent(NULL)	{  }
	~XMLNode( )				{ clear(); }

	XMLNode &operator=( XMLNode &prm );

	string	name( ) const			{ return mName; }
	XMLNode* setName( const string &s )	{ mName = s; return this; }

	string	text( ) const			{ return mText; }
	XMLNode* setText( const string &s )	{ mText = s; return this; }

	void	attrList( vector<string> &list ) const;
	XMLNode* attrDel( const string &name );
	void	attrClear( );
	string	attr( const string &name ) const;
	XMLNode* setAttr( const string &name, const string &val );

	void	prcInstrList( vector<string> & list ) const;
	void	prcInstrDel( const string &target );
	void	prcInstrClear( );
	string	prcInstr( const string &target ) const;
	XMLNode* setPrcInstr( const string &target, const string &val );

	void	load( const string &vl );
	string	save( unsigned flgs = 0 );
	XMLNode* clear( );

	bool	childEmpty( ) const		{ return mChildren.empty(); }
	int	childSize( ) const		{ return mChildren.size(); }
	void	childAdd( XMLNode *nd );
	XMLNode* childAdd( const string &name = "" );
	int	childIns( unsigned id, XMLNode *nd );
	XMLNode* childIns( unsigned id, const string &name = "" );
	void	childDel( const unsigned id );
	void	childDel( XMLNode *nd );
	void	childClear( const string &name = "" );
	XMLNode* childGet( const int, bool noex = false ) const;
	XMLNode* childGet( const string &name, const int numb = 0, bool noex = false ) const;
	XMLNode* childGet( const string &attr, const string &name, bool noex = false ) const;

	XMLNode* parent( )			{ return mParent; }

    private:
	//Methods
	void save( unsigned flg, string &xml );
	void encode( const string &s, string &rez, bool text = false ) const;

	static void start_element( void *data, const char *el, const char **attr );
	static void end_element( void *data, const char *el );
	static void characters( void *userData, const XML_Char *s, int len );
	static void instrHandler( void *userData, const XML_Char *target, const XML_Char *data );

	//Attributes
	string mName;
	string mText;
	vector< XMLNode* >		mChildren;
	vector< pair<string,string> >	mAttr;
	vector< pair<string,string> >	mPrcInstr;
	XMLNode *mParent;

	//> Parse/load XML attributes
	vector<XMLNode*> node_stack;
};

}

#endif // __XML_H
