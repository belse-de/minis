#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <cstdlib> // exit EXIT_* malloc perror

// unix system functions
#include <unistd.h>
#include <sys/wait.h> // waitpid

#ifdef UNUSED
#elif defined(__GNUC__)
#  define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
#  define UNUSED(x) /*@unused@*/ x
#else
#  define UNUSED(x) x
#endif

using namespace std;

struct mini_shell
{
    // Load config files, if any
    mini_shell();
    
    // Perform any shutdown/cleanup.
    ~mini_shell();
    
    // Run command loop.
    void loop();
    void execute( vector<string>& args );
    
    vector<string>& tokenize( const string& str, vector<string>& tokens, const string& delimiters );
    
    void cd( vector<string>& args );
    void exit( vector<string>& args );
    void lunch( vector<string>& args );
    void help( vector<string>& args );
    
    bool is_running;
};

int main( void  ) {
    cout << "Hello, user!" << endl;
    mini_shell sh;
    sh.loop();
    exit(EXIT_SUCCESS);
}

mini_shell::mini_shell() : is_running{true}
{
}

mini_shell::~mini_shell()
{
}

void mini_shell::loop()
{
    // FIXME constexpr
    const string token_delimiter = " \t\r\n\a";
    int status = 10;
    
    do
    {
        std::string current_line;
        getline(cin, current_line);
        
        vector<string> tokens;
        tokenize(current_line, tokens, token_delimiter);

        execute(tokens);
    } while(is_running);
}

void mini_shell::execute( vector<string>& args )
{
    if(args.size() > 0)
    {
        vector<string> striped_args(args.begin()+1, args.end());
        if( args[0] == "cd" )
        {
            cout << "if cd" << endl;
            cd(striped_args);
        }
        else if( args[0] == "exit" )
        {
            cout << "if exit" << endl;
            exit(striped_args);
        }
        else if( args[0] == "help" )
        {
            cout << "if help" << endl;
            help(striped_args);
        }
        else if( args[0] == "lunch" )
        {
            cout << "if lunch" << endl;
            lunch(striped_args);
        }
        else
        {
            cout << "lunch" << endl;
            lunch(args);
        }
    }
}

vector<string>& mini_shell::tokenize( const string& str, 
                            vector<string>& tokens,
                            const string& delimiters)
{
    // skip delimiters at the beginning of the string
    string::size_type last_pos = str.find_first_not_of(delimiters,0);
    // find first end of 'non-delimiters'
    string::size_type pos = str.find_first_of(delimiters, last_pos);
       
    while(pos != string::npos || last_pos != string::npos)
    {
        // we found a token, push it into the vector; substr expects a length
        string sub  = str.substr(last_pos, pos - last_pos);
        if(sub.length() > 0)
            tokens.push_back(sub);
        
        // skip next delimiters - new start of word
        last_pos = str.find_first_not_of(delimiters, pos);
        // find next end of 'non-delimiters'
        pos = str.find_first_of(delimiters, last_pos);
    }
    return tokens;
}

void mini_shell::cd( vector<string>& args )
{
    if(args.size() <= 0)
    {
        cerr << "Mini Sh: Expected argument to 'cd'" << endl;
        return;
    }
    
    if( chdir( args[1].c_str() ) != 0 )
    {
        perror("Mini Sh: could not change dir");
    }
}

void mini_shell::exit( vector<string>& UNUSED(args) )
{
    is_running = false;
}

void mini_shell::lunch( vector<string>& args )
{
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    //child
    if(pid == 0)
    {
        vector<char*> c_args;
        for(auto& arg : args)
        {
           c_args.push_back(&arg.front());
        }
        c_args.push_back(nullptr);
              
        if( execvp(c_args[0], c_args.data()) == -1)
        {
            perror("Mini Sh: could not start child process");
        }

        // exec returns only on failure
        ::exit(EXIT_FAILURE);
    }
    // error forking
    else if(pid < 0)
    {
        perror("Mini Sh: could not fork child process");
    }
    // parent
    else
    {
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED);
            if( wpid == -1 ) perror("Mini Sh: could not wait for child process");
        } while( not WIFEXITED(status) and  not WIFSIGNALED(status));
    }
}

void mini_shell::help( vector<string>& UNUSED(args) )
{
    cout <<
        "Build in commands: \n" <<
        "  cd [dir]\n" <<
        "  exit\n" <<
        "  lunch [exec]\n" <<
        endl;
}
