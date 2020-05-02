#include <dirent.h>
#include <iostream>
#include <string>
#include <string.h> // strsep, strlen, strcpy
#include <strings.h>
#include <unistd.h>
#include <vector>

void initTest( void );
void deinitTest( void );
bool GetCaseInsensitivePath( const std::string path, std::string & correctedPath);
static int original_casepath(char const *path, char *r);

void initTest( void ) {
    system("mkdir -p /tmp/fHoMm2tEsT");
    system("touch /tmp/fHoMm2tEsT/fileAccesS.test");
    system("mkdir -p fHoMm2tEsT");
    system("touch fHoMm2tEsT/filEacCeSs.test");
    system("mkdir -p Герои2\\ на\\ свободном\\ движке");
    system("rm -r fhomm3test"); // for guarantee that directory "fhomm3test" is absent
    system("rm -r ГероиМечаИМагии2"); // for guarantee that directory "ГероиМечаИМагии2" is absent
}

void deinitTest( void ) {
    system("rm -r /tmp/fHoMm2tEsT");
    system("rm -r fHoMm2tEsT");
    system("rm -r Герои2\\ на\\ свободном\\ движке");
}

int main(){
    initTest();
	std::vector<std::pair<std::string, bool>> testVectors = {
                                        {"/tmp/fhomm2test", true},
                                        {"/tmp/fhomm2test/fileaccess.test", true},
                                        {"./fhomm2teSt", true},
                                        {"./fhoMm2test/", true},
                                        {"./fhoMm2test/fileaccess.test", true},
                                        {"./fhomm3test", false},
                                        {"fhomm2test", true},
                                        {"ГероиМечаИМагии2", false},
                                        {"ГеРоИ2 на свОбоДном движке", true},
                                        {"Герои2 на свободном движке", true}
                                        };

    // original_casepath test that show bug
    // char *path = "./fhomm3test"; // this path is inaccessible, because above in initTest() we do "rm -r fhomm3test"
    // char *path = "ГероиМечаИМагии2"; // this path is inaccessible, because in initTest we do "rm -r ГероиМечаИМагии2"
    // char *r = (char *)alloca(strlen(path) + 2);
    // if (original_casepath(path, r))
    // {
    //     std::cout << r << "is accessible";
    // }
    // else {
    //     std::cout << path << "not accessible";
    // }

    std::cout << "Current working directory: " << getcwd( 0, 0 ) << std::endl;
    std::string correctedPath; // let it be outside for cycle, need to check that it always resets to empty string
	for (std::size_t iterPath = 0; iterPath < testVectors.size(); ++iterPath) {
        if ( GetCaseInsensitivePath( testVectors[iterPath].first, correctedPath ) != testVectors[iterPath].second ) {
            std::cerr << "[" << iterPath << "] [FAIL] Check failed for path: " <<
                testVectors[iterPath].first << " awaiting " << (testVectors[iterPath].second?"true":"false") << std::endl;
        }
        else {
            if ( testVectors[iterPath].second ) {
                std::cout << "[" << iterPath << "] [SUCCESS] Case insensitive path found: " <<
                    correctedPath << std::endl;
            }
            else {
                std::cout << "[" << iterPath << "] [SUCCESS] Case insensitive path not accessible: " <<
                    testVectors[iterPath].first << std::endl;
            }
        }
	}

    deinitTest();

	return 0;
}

// splitString - function for splitting strings by delimiter
std::vector<std::string> splitString( std::string path, std::string delimiter )
{
    std::vector<std::string> result;

    if ( path.empty() ) {
        return result;
    }

    size_t nPos = path.find( delimiter, 0 );
    while ( nPos != std::string::npos ) { // while found delimiter
        size_t nnPos = path.find( delimiter, nPos + 1 );
        if ( nnPos != std::string::npos ) { // if found next delimiter
            result.push_back( path.substr( nPos + 1, nnPos - nPos - 1 ) );
        }
        else { // if no more delimiter present
            if ( !path.substr( nPos + 1 ).empty() ) { // if not a postfix delimiter
                result.push_back( path.substr( nPos + 1 ) );
            }
        }

        nPos = path.find( delimiter, nPos + 1 );
    }

    if ( result.empty() ) { // if delimiter not present
        result.push_back( path );
    }

    return result;
}

// strcasecmp_u - function for case-insensitive string comparement
// taked here: https://pastebin.com/f38bac88d
// explanations(in Russian) in this topic https://www.linux.org.ru/forum/development/4106230?cid=4119131
int strcasecmp_u( const char *s1, const char *s2 ) {
#define mbstowcsmacro( s, w, l ) \
    l = mbstowcs( NULL, s, 0 ) + 1; \
    w = ( wchar_t* ) calloc( l, sizeof( wchar_t ) ); \
    mbstowcs( w, s, l );

#define str_u_epilog( w1, w2 ) \
    free( w2 ); \
    free( w1 );

    int l1, l2, result;
    wchar_t *w1, *w2;
    mbstowcsmacro( s1, w1, l1 );
    mbstowcsmacro( s2, w2, l2 );
    result = wcscasecmp( w1, w2 );
    str_u_epilog( w1, w2 );
    return result;
};

bool GetCaseInsensitivePath( const std::string path, std::string & correctedPath )
{
    DIR * d;
    bool last = false;
    correctedPath.clear();

    const char chCurDir = '.';
    const char * strCurDir = &chCurDir;
    const char chDelimiter = '/';
    const char * strDelimiter = &chDelimiter;

    if ( path.empty() )
        return false;

    if ( path[0] == chDelimiter ) {
        d = opendir( strDelimiter );
    }
    else {
        correctedPath = chCurDir;
        d = opendir( strCurDir );
    }

    std::vector<std::string> splittedPath = splitString( path, strDelimiter );
    for ( std::vector<std::string>::iterator subPathIter = splittedPath.begin(); subPathIter != splittedPath.end(); ++subPathIter ) {
        if ( !d ) {
            return false;
        }

        if ( last ) {
            closedir( d );
            return false;
        }

        correctedPath.append( strDelimiter );

        struct dirent * e = readdir( d );
        while ( e ) {
            if ( strcasecmp( ( *subPathIter ).c_str(), e->d_name ) == 0 ) {
                correctedPath += e->d_name;

                closedir( d );
                d = opendir( correctedPath.c_str() );

                break;
            }

            e = readdir( d );
        }

        if ( !e ) {
            correctedPath += *subPathIter;
            last = true;
        }
    }

    if ( d )
        closedir( d );

    return !last;
}

// taked here: https://github.com/OneSadCookie/fcaseopen/blob/master/fcaseopen.c
// r must have strlen(path) + 2 bytes
static int original_casepath(char const *path, char *r)
{
    size_t l = strlen(path);
    char *p = (char *)alloca(l + 1);
    strcpy(p, path);
    size_t rl = 0;
    
    DIR *d;
    if (p[0] == '/')
    {
        d = opendir("/");
        p = p + 1;
    }
    else
    {
        d = opendir(".");
        r[0] = '.';
        r[1] = 0;
        rl = 1;
    }
    
    int last = 0;
    char *c = strsep(&p, "/");
    while (c)
    {
        if (!d)
        {
            return 0;
        }
        
        if (last)
        {
            closedir(d);
            return 0;
        }
        
        r[rl] = '/';
        rl += 1;
        r[rl] = 0;
        
        struct dirent *e = readdir(d);
        while (e)
        {
            if (strcasecmp(c, e->d_name) == 0)
            {
                strcpy(r + rl, e->d_name);
                rl += strlen(e->d_name);

                closedir(d);
                d = opendir(r);
                
                break;
            }
            
            e = readdir(d);
        }
        
        if (!e)
        {
            strcpy(r + rl, c);
            rl += strlen(c);
            last = 1;
        }
        
        c = strsep(&p, "/");
    }
    
    if (d) closedir(d);
    return 1;
}