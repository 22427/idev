
#include <fstream>

#include <sstream>
#include <unordered_map>
#include <vector>
#include <iostream>

#define IDEV_KEYBOARD_EVENTS "120013"
#define IDEV_MOUSE_EVENTS "17"
#define IDEV_B_EVENTS "EV"

namespace idev {

typedef std::string String;
class Device
{
public:
	//I: ...
	struct I_data
	{
		//Bus=<Bus>
		String Bus;
		//Vendor=<Vendor>
		String Vendor;
		//Product=<Product>
		String Product;
		//Version=<Version>
		String Version;
	} I;
	//N: Name=<N>
	String N;
	//P: Phys=<P>
	String P;
	//S: Sysfs=<S>
	String S;
	//U: Uniq=<U>
	String U;
	//H: Handlers=<H>
	std::vector<String> H;
	//B: <first>=<second>
	std::unordered_map<String,String> B;
	
	// parse the s**t out of it. Not the most reliable parser, not even close,
	// but it works for now!
	// returns true if there is more to read!
	bool read(std::ifstream& f)
	{
		String line;
		while(std::getline(f,line))
		{
			if(line[0] == 'I')
			{
				line = line.substr(line.find_first_of('=')+1);
				I.Bus = line.substr(0,line.find_first_of(' '));
				line = line.substr(line.find_first_of('=')+1);
				I.Vendor = line.substr(0,line.find_first_of(' '));
				line = line.substr(line.find_first_of('=')+1);
				I.Product = line.substr(0,line.find_first_of(' '));
				line = line.substr(line.find_first_of('=')+1);
				I.Version = line.substr(0,line.find_first_of(' '));
			}
			else if(line[0] == 'N')
			{
				line = line.substr(line.find_first_of('=')+1);
				N = line;
			}
			else if(line[0] == 'P')
			{
				line = line.substr(line.find_first_of('=')+1);
				P = line;
			}
			else if(line[0] == 'S')
			{
				line = line.substr(line.find_first_of('=')+1);
				S = line;
			}
			else if(line[0] == 'U')
			{
				line = line.substr(line.find_first_of('=')+1);
				U = line;
			}
			else if(line[0] == 'H')
			{
				line = line.substr(line.find_first_of('=')+1);
				std::stringstream ss(line);
				String h;
				while(getline(ss,h,' '))
				{
					H.push_back(h);
				}
			}
			else if(line[0] == 'B')
			{
				line = line.substr(line.find_first_of(' ')+1);
				String name = line.substr(0,line.find_first_of('='));
				String valu = line.substr(line.find_first_of('=')+1);
				B[name] = valu;
			}
			else
				break;
		}
		return !f.eof();
	}
	
	// converts this Device back to the string it was parsed from.
	String to_string() const
	{
		String res = "I: ";
		res+="Bus="+I.Bus+" Vendor="+I.Vendor+" Product="+
				I.Product+" Version="+I.Version+"\n";
		res+="N: Name="+N+"\n";
		res+="P: Phys="+P+"\n";
		res+="S: Sysfs="+S+"\n";
		res+="U: Uniq="+U+"\n";
		res+="H: Handlers=";
		for(const auto& h : H)
		{
			res+=h+" ";
		}
		res+="\n";
		for(const auto& b : B)
		{
			res+="B: "+b.first+"="+b.second+"\n";
		}
		res+="\n";
		return res;
	}
	
	// searches for an handler starting with <starts_with> and returns it.
	// if none is found an empty string is returned.
	String get_handler_starting_with(const String& starts_with) const
	{
		for(const auto& h : H)
		{
			if(h.find_first_of(starts_with) == 0)
				return h;
		}
		return "";
	}
};


//filters device list, searching for devices where the "B" attribute attrib
//matches value
std::vector<Device> filter_B(const std::vector<Device>& in,
							 const String& attrib,
							 const String& value)
{
	std::vector<Device> out;
	for(const auto& d : in)
	{
		auto it = d.B.find(attrib);
		if(it != d.B.end() && (*it).second == value)
			out.push_back(d);
	}
	return out;
}

// reads all devices from proc/bus/input/devices
std::vector<Device> get_devices(const String& p="/proc/bus/input/devices")
{
	std::ifstream f;
	f.open(p);
	if(!f.is_open())
		std::cout<<"cannot open /proc/bus/input/devices"<<std::endl;
	std::vector<Device> res;
	Device dev;
	dev.B.clear();
	dev.H.clear();
	while(dev.read(f))
	{
		res.push_back(dev);
		dev.B.clear();
		dev.H.clear();
	}
	
	f.close();
	return  res;
}

std::vector<Device> get_keyboards(const String& p="/proc/bus/input/devices")
{
	auto devs = get_devices(p);
	return  filter_B(devs,IDEV_B_EVENTS,IDEV_KEYBOARD_EVENTS);
}

std::vector<Device> get_mice(const String& p = "/proc/bus/input/devices")
{
	auto devs = get_devices(p);
	return  filter_B(devs,IDEV_B_EVENTS,IDEV_MOUSE_EVENTS);
}

}

//Example 
#if 0
int main(int /*argc*/, char** /*argv*/)
{
	auto keyboards = idev::get_keyboards();
	auto mice = idev::get_mice();
	
	std::cout<<"keyboards:\n";
	for(const auto& k : keyboards)
	{
		std::cout<<"\t"<<k.get_handler_starting_with("event")<<"\n";
	}
	std::cout<<"mice:\n";
	for(const auto& m : mice)
	{
		std::cout<<"\t"<<m.get_handler_starting_with("event")<<"\n";
	}
	
	return 0;
}
#endif
