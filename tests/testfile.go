// The english wiki page had not such nice example
// https://de.wikipedia.org/wiki/Go_(Programmiersprache)#Objektorientierung
package main

import "fmt"

// Definieren zweier Typen
type User struct {
	Name string
}

type Admin struct {
	User  // Admin bettet zudem den Typ 'User' ein
	Email string
}

// Ein Interface mit der Methode 'Notify()'
type Notifier interface {
	Notify()
}

// User und Admin implementieren das Interface 'Notifier', indem sie jeweils eine
// entsprechende 'Notify()'-Methode haben. Eine vorherige Deklaration zur Implementierung
// ist nicht notwendig.

func (u User) Notify() {
	fmt.Printf("User: Sending user email to %s\n", u.Name)
}

func (a Admin) Notify() {
	fmt.Printf("Admin: Sending admin email to %s. The email address is %q.\n",
		a.Name, // Verwenden des eingebetteten Feldes 'Name' vom User
		a.Email)
}

func main() {
	// Einen User und einen Admin erstellen
	user := User{
		Name: "john smith",
	}
	admin := Admin{
		User:  user,
		Email: "john@example.com",
	}

	// Eine Variable vom Interfacetyp anlegen
	var n Notifier

	// Die implementierte Notify-Methode aufrufen
	// Die Methode wird über die Interfacevariable aufgerufen
	// Mittels dynamischer Bindung wird die Methode am richtigen Typ aufgerufen
	n = user
	n.Notify()
	n = admin
	n.Notify()
}

// Some arbitary stuff, from english wiki
// ZeroBroadcast reports whether addr is 255.255.255.255.
type ipv4addr uint32

func (addr ipv4addr) ZeroBroadcast() bool {
	return addr == 0xFFFFFFFF
}

// Cuts from pacoloco project
type Downloader struct {

}

func (d *Downloader) decrementUsage() {
	val := d.usageCount.Add(-1)
}

func getDownloadReader(f *RequestedFile) (time.Time, io.ReadSeekCloser, error) {
	d, err := getDownloader(f)

	return time.Time{}, nil, d.eventError
}
