#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

class HashEntry
{
public:
    HashEntry(string key, int offset):m_key(key), m_next(NULL), m_offset(offset){}
    ~HashEntry();
    string key() { return m_key;}
    int offset() { return m_offset;}
    HashEntry* m_next;
private:
    string m_key;
    int m_offset;
    
    
};
unsigned int TableSize=200000;
class HashTable
{
public:
    HashTable()
    {
        table = new HashEntry*[TableSize];
        for(int i=0; i<TableSize; i++)
        {
            table[i]=NULL;
        }
    }
    unsigned int hash(string key)   //FNV-1 Algorithm
    {
        unsigned int h=2166136261;
        for (int k=0; k!=key.size(); k++)
        {
            h += key[k];
            h *= 16777619;
        }
        return h;
    }
    void add(string key, int offset)
    {
        unsigned int h=hash(key) % TableSize;
        if(table[h]==NULL)
        {
            table[h]=new HashEntry(key, offset);
        }
        else
        {
            HashEntry* entry= table[h];
            while(entry!=NULL)
            {
                entry=entry->m_next;
            }
            entry=new HashEntry(key, offset);
        }
    }
    int get(string key)
    {
        unsigned int h=hash(key) % TableSize;
        if(table[h]==NULL)  //if it's empty return -1
            return -1;
        if (table[h]->key()==key)
        {
            return table[h]->offset(); 
        }
        else
        {
            HashEntry* entry = table[h];
            while(entry->key()!=key)
            {
                if (entry->m_next == NULL)
                    return -1;
                entry=entry->m_next;
            }
            return entry->offset();
        }
    }
private:
    HashEntry **table;
};


void createDelta(istream& oldf, istream& newf, ostream& deltaf)
{
    string os="";  //string for old file
    while (!oldf.eof())
	{   string temp;
		getline(oldf,temp);
		os+=temp;
        if(!oldf.eof())
            os+='\n';
	}
    
    //cerr<<os;
    string ns="";  //string for new file
    while (!newf.eof())
	{
        string temp;
		getline(newf,temp);
		ns+=temp;
        if(!newf.eof())
            ns+= '\n';
	}
    HashTable Table;
    for(int i=0; i<os.size(); i++)      //create a Hash Table with 8 characters in each position
    {
        string entry=os.substr(i,8);
        Table.add(entry, i);
    }
//    int j=0;
    
    for(int j=0; j<ns.size();)
    {
        string target=ns.substr(j, 8);
        
        int offset=Table.get(target);
        if(offset!=-1) //found it
        {
            int L=0;
            while(ns[j+L]==os[offset+L])
            {
                L++;
                if(j+L>=ns.size() || offset+L>=os.size())
                    break;
            }
            
            deltaf<<"C"<<L<<","<<offset;    //print copy command to delta file
            j+=L;
        }
        
        else    //if target not found in hash table, add new characters
        {
            int L=0;
            string target=(ns.substr(j+L, 8));
            while(Table.get(target)==-1 && j+L<ns.size())
            {
                L++;
                target=(ns.substr(j+L, 8));
            }
            deltaf<<"A"<<L<<":"<<ns.substr(j, L);   //print add command to delta file
            j+=L;
        }
        
    }
};

bool applyDelta(istream& oldf, istream& deltaf, ostream& newf)
{
    string os="";  //string for old file
    while (!oldf.eof())
	{   string temp;
		getline(oldf,temp);
		os+=temp;
        if(!oldf.eof())
            os+='\n';
	}
    
    
    string deltastring; //string for delta file
    while(!deltaf.eof())
    {
        string temp;
        getline(deltaf, temp);
        deltastring+=temp+'\n';
    }
    //cerr<<deltastring;
    
    
    for(int i=0; i<deltastring.size();)
    {
        if(deltastring[i]=='A') //if add command
        {
            int length=0;
            while(deltastring[i+1+length]!=':')
            {
                length++;
            }
            int temp=length;
            int p=i;
            int result=0;
            while(deltastring[p+1]!=':')
            {
                char target=deltastring[p+1];
                int targetint = target-'0';
                for(int k=0;k<temp-1;k++)
                {
                    targetint*=10;
                }
                result+=targetint;
                temp--;
                p++;
            }
        
            string output=deltastring.substr(i+2+length, result);
            newf<<output;
            i+=2+length+result;
        }
        
        else if(deltastring[i]=='C')    //if copy command
        {
            int lengthBeforeComma=0;
            while(deltastring[i+1+lengthBeforeComma]!=',')
            {
                lengthBeforeComma++;
            }
            int temp=lengthBeforeComma;
            int p=i;
            int amountToCopy=0;
            while(deltastring[p+1]!=',')
            {
                char target=deltastring[p+1];
                int targetint = target-'0';
                for(int k=0;k<temp-1;k++)
                {
                    targetint*=10;
                }
                amountToCopy+=targetint;
                temp--;
                p++;
                if(p+1>=deltastring.size())
                    break;
            }
            
            
            int lengthAfterComma=0;
            while(i+lengthBeforeComma+2+lengthAfterComma<deltastring.size()&&
                  deltastring[i+lengthBeforeComma+2+lengthAfterComma]!='A' &&
                  deltastring[i+lengthBeforeComma+2+lengthAfterComma]!='C' &&
                  deltastring[i+lengthBeforeComma+2+lengthAfterComma]!='\n')
            {
                lengthAfterComma++;
            }
            int temp2=lengthAfterComma;
            int q=i;
            int position=0;
            while(i+lengthBeforeComma+2+temp2<deltastring.size()&&
                  deltastring[lengthBeforeComma+2+q]!='A' &&
                  deltastring[lengthBeforeComma+2+q]!='C' &&
                  deltastring[lengthBeforeComma+2+q]!='\n'
                  )
            {
                char target=deltastring[lengthBeforeComma+2+q];
                int targetint = target - '0';
                for (int k=0; k<temp2-1; k++)
                {
                    targetint*=10;
                }
                position+=targetint;
                temp2--;
                q++;
                if(lengthBeforeComma+2+q>=deltastring.size())
                    break;
            }
            
            string output=os.substr(position, amountToCopy);
            newf<<output;
            i+=lengthBeforeComma+2+lengthAfterComma;
        }
        
        
        else if(i>=deltastring.size()-1)
            return true;
        else
            return false;
    
    }
    return true;
}



int main()
{
    ifstream oldf("/Users/Gageek/Dropbox/Computer Science/CS32/CS32 Project 4/p4test/smallmart1.txt");
    if (!oldf)
    {
        cerr<<"ERROR: Cannot access file1";
    }
    
    ifstream newf("/Users/Gageek/Dropbox/Computer Science/CS32/CS32 Project 4/p4test/smallmart2.txt");
    if( !newf)
    {
        cerr<<"ERROR: Cannot access file2";
    }
    
    ofstream deltaf("/Users/Gageek/Dropbox/Computer Science/CS32/CS32 Project 4/p4test/smallmartdelta.txt");
    if( !deltaf)
    {
        cerr<<"ERROR: Delta file failed";
    }
    oldf.clear();
    oldf.seekg(0);
  createDelta(oldf, newf, deltaf);
   //return 0;
    
    oldf.clear();
    oldf.seekg(0);
    deltaf.seekp(0);
    
    ifstream deltaf2("/Users/Gageek/Dropbox/Computer Science/CS32/CS32 Project 4/p4test/smallmartdelta.txt");
    deltaf2.seekg(0);
    
    ofstream newfromdelta("/Users/Gageek/Dropbox/Computer Science/CS32/CS32 Project 4/p4test/Mysmallmart.txt");
    if (!newfromdelta)
    {
        cerr<<"ERROR1";
    }

    applyDelta(oldf, deltaf2, newfromdelta);
    return 0;
}
