#include "excep.h"
#include "xml.h"

using namespace SCADA;
//*************************************************
//* XMLNode                                       *
//*************************************************
XMLNode &XMLNode::operator=(XMLNode &prm)
{
    //> Delete self children and attributes
    mAttr.clear();
    mPrcInstr.clear();
    for( unsigned int i_ch = 0; i_ch < mChildren.size(); i_ch++ )
        delete mChildren[i_ch];
    mChildren.clear();

    //> Copy params (name,text, attributes and instructions)
    setName( prm.name() );
    setText( prm.text() );
    vector<string> ls;
    prm.attrList(ls);
    for( unsigned int i_a = 0; i_a < ls.size(); i_a++)
        setAttr(ls[i_a],prm.attr(ls[i_a]));
    prm.prcInstrList(ls);
    for( unsigned int i_p = 0; i_p < ls.size(); i_p++)
        setPrcInstr(ls[i_p],prm.prcInstr(ls[i_p]));

    //> Recursive copy children
    for( int i_ch = 0; i_ch < prm.childSize(); i_ch++ )
        *childAdd() = *prm.childGet(i_ch);

    return *this;
}

void XMLNode::childAdd( XMLNode *n )
{
    if( !n )
        return;
    mChildren.push_back(n);
    n->mParent = this;
}

XMLNode* XMLNode::childAdd( const string &name )
{
    XMLNode *n = new XMLNode( name );
    childAdd(n);

    return n;
}

void XMLNode::childDel( const unsigned id )
{
    if( id >= (unsigned int)childSize() )
        throw TError("XMLNode","Child %d is not present.",id);
    delete mChildren[id];
    mChildren.erase( mChildren.begin()+id );
}

void XMLNode::childDel( XMLNode *nd )
{
    for( unsigned int i_ch = 0; i_ch < mChildren.size(); i_ch++ )
        if( mChildren[i_ch] == nd )
        {
            delete mChildren[i_ch];
            mChildren.erase( mChildren.begin()+i_ch );
            break;
        }
}

void XMLNode::childClear( const string &name )
{
    for( unsigned int i_ch = 0; i_ch < mChildren.size(); i_ch++ )
        if( name.empty() || mChildren[i_ch]->name() == name )
            childDel(i_ch--);
}

int XMLNode::childIns( unsigned id, XMLNode * n )
{
    if( !n )
        return -1;

    if( id > (unsigned int)childSize() )
        id = childSize();
    mChildren.insert( mChildren.begin()+id, n );
    n->mParent = this;

    return id;
}

XMLNode* XMLNode::childIns( unsigned id, const string &name )
{
    XMLNode *n = new XMLNode( name );
    childIns(id,n);

    return n;
}

XMLNode* XMLNode::childGet( const int index, bool noex ) const
{
    if( index >= 0 && index < childSize() )
        return mChildren[index];
    if( noex )
        return NULL;
    throw TError("XMLNode","Child %d is not present.",index);
}

XMLNode* XMLNode::childGet( const string &name, const int numb, bool noex ) const
{
    for( int i_ch = 0, i_n = 0; i_ch < childSize(); i_ch++ )
        if( childGet(i_ch)->name() == name && i_n++ == numb )
            return childGet(i_ch);

    if( noex )
        return NULL;
    throw TError("XMLNode","Child %s:%d is not found!",name.c_str(),numb);
}

XMLNode* XMLNode::childGet( const string &attr, const string &val, bool noex ) const
{
    for( unsigned i_f = 0; i_f < (unsigned int)childSize(); i_f++ )
        if( childGet(i_f)->attr(attr) == val )
            return childGet(i_f);

    if( noex )
        return NULL;
    throw TError("XMLNode","Child with attribut %s=%s is not present.",attr.c_str(),val.c_str());
}

void XMLNode::attrList( vector<string> & list ) const
{
    list.clear();
    for( unsigned i_a = 0; i_a < mAttr.size(); i_a++ )
        list.push_back(mAttr[i_a].first);
}

XMLNode* XMLNode::attrDel( const string &name )
{
    for( unsigned i_a = 0; i_a < mAttr.size(); i_a++ )
        if( mAttr[i_a].first == name )
        {
            mAttr.erase(mAttr.begin()+i_a);
            break;
        }

    return this;
}

void XMLNode::attrClear( )
{
    mAttr.clear();
}

string XMLNode::attr( const string &name ) const
{
    for( unsigned i_a = 0; i_a < mAttr.size(); i_a++ )
        if( mAttr[i_a].first == name )
            return mAttr[i_a].second;

    return "";
}

XMLNode* XMLNode::setAttr( const string &name, const string &val )
{
    for( unsigned i_a = 0; i_a < mAttr.size(); i_a++ )
        if( mAttr[i_a].first == name )
        {
            mAttr[i_a].second = val;
            return this;
        }

    mAttr.push_back(pair<string,string>(name,val));

    return this;
}

void XMLNode::prcInstrList( vector<string> & list ) const
{
    list.clear();
    for( unsigned i_p = 0; i_p < mPrcInstr.size(); i_p++ )
        list.push_back(mPrcInstr[i_p].first);
}

void XMLNode::prcInstrDel( const string &target )
{
    for( unsigned i_p = 0; i_p < mPrcInstr.size(); i_p++ )
        if( mPrcInstr[i_p].first == target )
        {
            mPrcInstr.erase(mPrcInstr.begin()+i_p);
            return;
        }
}

void XMLNode::prcInstrClear( )
{
    mPrcInstr.clear();
}

string XMLNode::prcInstr( const string &target ) const
{
    for( unsigned i_p = 0; i_p < mPrcInstr.size(); i_p++ )
        if( mPrcInstr[i_p].first == target )
            return mPrcInstr[i_p].second;

    return "";
}

XMLNode* XMLNode::setPrcInstr( const string &target, const string &val )
{
    for( unsigned i_p = 0; i_p < mPrcInstr.size(); i_p++ )
        if( mPrcInstr[i_p].first == target )
        {
            mPrcInstr[i_p].second = val;
            return this;
        }

    mPrcInstr.push_back(pair<string,string>(target,val));

    return this;
}

XMLNode* XMLNode::clear()
{
    attrClear();
    setText("");
    prcInstrClear();
    childClear();

    return this;
}

string XMLNode::save( unsigned flg )
{
    string xml;
    xml.reserve(10000);

    if( flg&XMLHeader )
    {
        xml += "<?xml version='1.0' encoding='UTF-8' ?>";
        if( flg&XMLNode::BrClosePast )
            xml += "\n";
    }

    save(flg,xml);
    return xml;
}

void XMLNode::save( unsigned flg, string &xml )
{
    xml.append( (flg&XMLNode::BrOpenPrev && xml.size() && xml[xml.size()-1] != '\n') ? "\n<" : "<" );
    if( flg&XMLNode::MissTagEnc )
        xml.append( name() );
    else
        encode( name(), xml );

    for( unsigned i_a = 0; i_a < mAttr.size(); i_a++ )
    {
        xml.append(" ");
        if( flg&XMLNode::MissAttrEnc )
            xml.append( mAttr[i_a].first );
        else
            encode( mAttr[i_a].first, xml );
        xml.append("=\"");
        encode( mAttr[i_a].second, xml );
        xml.append("\"");
    }

    if( childEmpty() && text().empty() && mPrcInstr.empty() )
        xml.append( (flg&(XMLNode::BrOpenPast|XMLNode::BrClosePast)) ? "/>\n" : "/>" );
    else
    {
        xml.append( (flg&XMLNode::BrOpenPast) ? ">\n" : ">" );
        //> Save text
        if( !text().empty() )
        {
            encode( text(), xml, true );
            xml.append( flg&XMLNode::BrTextPast ? "\n" : "" );
        }
        //> Save process instructions
        for( unsigned int i_p = 0; i_p < mPrcInstr.size(); i_p++ )
            xml.append( "<?"+mPrcInstr[i_p].first+" "+mPrcInstr[i_p].second+(flg&XMLNode::BrPrcInstrPast?"?>\n":"?>") );
        //> Save included childs
        for( int i_c = 0; i_c < childSize(); i_c++ )
            childGet(i_c)->save(flg,xml);
        //> Close tag
        xml.append("</");
        if( flg&XMLNode::MissTagEnc )
            xml.append(name() );
        else
            encode( name(), xml );
        xml.append( flg&XMLNode::BrClosePast ? ">\n" : ">" );
    }
}

void XMLNode::encode( const string &s, string &rez, bool text ) const
{
    const char *replStr = NULL;
    for( unsigned int i_sz = 0, f_pos = 0; true; )
    {
        switch( s[i_sz] )
        {
        case '>':
            replStr = "&gt;";
            break;
        case '<':
            replStr = "&lt;";
            break;
        case '"':
            replStr = "&quot;";
            break;
        case '&':
            replStr = "&amp;";
            break;
        case '\'':
            replStr = "&#039;";
            break;
        case '\n':
            if( !text )
                replStr = "&#010;";
            break;
        }
        i_sz++;
        if( replStr )
        {
            if( (i_sz-1) > f_pos )
                rez.append(s,f_pos,i_sz-f_pos-1);
            rez.append(replStr);
            replStr = NULL;
            f_pos = i_sz;
        }
        if( i_sz >= s.size() )
        {
            if( i_sz > f_pos )
                rez.append(s,f_pos,i_sz-f_pos);
            break;
        }
    }
}

void XMLNode::load( const string &s )
{
    clear();

    XML_Parser p = XML_ParserCreate("UTF-8");
    if( !p )
        throw TError("XMLNode","Couldn't allocate memory for parser.");

    XML_SetElementHandler( p, start_element, end_element );
    XML_SetCharacterDataHandler( p, characters );
    XML_SetProcessingInstructionHandler( p, instrHandler );
    XML_SetUserData( p, this );

    XMLNode *lstParent = mParent;
    mParent = NULL;

    if( !XML_Parse( p, s.c_str(), s.size(), true ) )
    {
        int cL = XML_GetCurrentLineNumber(p);
        string xmlErr = XML_ErrorString(XML_GetErrorCode(p));
        XML_ParserFree( p );
        mParent = lstParent;
        throw TError("XMLNode","Parse error at line %d --- %s. Source string: '%s'", cL, xmlErr.c_str(), ((s.size()>1024)?s.substr(0,1024)+"...":s).c_str());
    }
    XML_ParserFree( p );
    mParent = lstParent;
}

//> Parse/load XML attributes
void XMLNode::start_element( void *data, const char *el, const char **attr )
{
    const char *a_n, *a_v;
    XMLNode *p = (XMLNode*)data;
    XMLNode *n = p;

    if( p->mParent )
        n = p->mParent->childAdd();

    n->setName(el);
    while( *attr )
    {
        a_n = *attr++;
        a_v = *attr++;
        n->mAttr.push_back(pair<string,string>(a_n,a_v));
    }

    p->node_stack.push_back(n);
    p->mParent = n;
}

void XMLNode::end_element( void *data, const char *el )
{
    XMLNode *p = (XMLNode*)data;

    //> Remove spaces from end of text
    int i_ch;
    for( i_ch = p->mParent->mText.size()-1; i_ch >= 0; i_ch--)
        if( !(p->mParent->mText[i_ch] == ' ' || p->mParent->mText[i_ch] == '\n' || p->mParent->mText[i_ch] == '\t') )
            break;
    //> Encode text
    p->mParent->setText(p->mParent->mText.substr(0,i_ch+1));

    if( !p->node_stack.empty() )
        p->node_stack.pop_back();
    if( p->node_stack.empty() )
        p->mParent = NULL;
    else
        p->mParent = p->node_stack[p->node_stack.size()-1];
}

void XMLNode::characters( void *userData, const XML_Char *s, int len )
{
    XMLNode *p = ((XMLNode*)userData)->mParent;
    //if( !len )	return;
    if( p->mText.size() )
        p->mText.append(s,len);
    else
        for(int i_ch = 0; i_ch < len; i_ch++)
        {
            if(s[i_ch] == ' ' || s[i_ch] == '\n' || s[i_ch] == '\t' )
                continue;
            p->mText.assign(s+i_ch,len-i_ch);
            break;
        }
}

void XMLNode::instrHandler( void *userData, const XML_Char *target, const XML_Char *data )
{
    ((XMLNode*)userData)->mParent->mPrcInstr.push_back(pair<string,string>(target,data));
}

