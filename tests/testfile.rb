
# https://de.wikipedia.org/wiki/Ruby_(Programmiersprache)
# Definiere „Funktion“
def meine_funktion
  puts "Hier bin ich"
end

# Kann in andern Objekten wie eine Funktion benutzt werden
class ABC
  def gib_meine_funktion_aus
    meine_funktion
  end
end
mein_abc = ABC.new
mein_abc.gib_meine_funktion_aus # => Hier bin ich
mein_abc.meine_funktion # => Fehlermeldung, private method `meine_funktion' called

auto1 = Object.new
def auto1.beschleunigen
  puts "brumm brumm brumm"
end

auto1.beschleunigen # => "brumm brumm brumm"

auto2 = auto1.clone
def auto2.bremsen
  puts "quietsch"
end

auto2.beschleunigen # => "brumm brumm brumm"
auto2.bremsen       # => "quietsch"

10.times {
  puts "Hallo Welt!"
}
10.times do
  puts "Hallo Welt!"
end


def methode_die_block_aufruft(übergebener_parameter)
  eigene_variable = "Hallo"
  rückgabe = yield eigene_variable, übergebener_parameter
  if rückgabe == "ok"
    puts "☺"
  end
end

# Aufruf
methode_die_block_aufruft("aus Ruby") do |p1,p2| # die Blockargumente werden innerhalb || in block-lokale Variablen umgewandelt
  puts p1 + " " + p2 + "!"
  "ok"
end
# Zuerst wird im Block „Hallo aus Ruby!“ ausgegeben,
# dann in der Methode ☺, da der Rückgabewert „ok“ war


def mache_block_zu_proc &block
  block # ist der Rückgabewert, da letzter Ausdruck
end
a = mache_block_zu_proc{|a,b| a + b}
b = proc {|a,b| a - b} # ein return innerhalb des Blocks verlässt die beinhaltende Methode, break nur den Block
c = lambda {|a,b| a * b} # wie proc, aber return innerhalb des lamdas verlässt nur diesen Block
d = -> (a,b) {a / b} # neue Schreibweise für lambda, runde Klammern sind optional
a.call(1,2) # => 3
b[1,2]      # => -1
c.(1,2)     # => 2

# Beispiel für funktionale Programmierung
e = d.curry.(8) # neues Proc das den (oder die) ersten Parameter (den Dividenden hier) als 8 setzt
e.(2)       # => 4
# mit & können Proc-Objekte (eigentlich jede Klasse die eine call Methode hat) wieder in Blöcke umgewandelt werden
[8,4,2,1].map(&e) # => [1, 2, 4, 8]



class Tier
  def sagt
    puts "#{self.class} sagt nichts" # "text#{logik}text" ist Rubys Textinterpolation
  end
end

module KannSchwimmen # Module, d. h. KannSchwimmen.new geht nicht
  def schwimmt
    puts "#{self.class} schwimmt" # self gibt die Instanz zurück,
  end                             #  jedes Objekt hat eine Methode .class die das Klassenobjekt zurück gibt
end

module KannNichtSchwimmen
  def schwimmt
    puts "#{self.class} geht unter"
  end
end

class Fisch < Tier # Vererbung wird durch den Kleiner-Als-Operator gekennzeichnet
  include KannSchwimmen # füge KannSchwimmen zwischen Fisch und Tier ein
end

class Vogel < Tier
  include KannNichtSchwimmen
  def sagt # übliches Vererben, überdecke sagt-Methode von Tier
    puts "#{self.class}: Piep"
  end
end

class Mensch < Tier
  include KannSchwimmen
  def sagt
    puts "#{self.class}: Ich kann mich besser ausdrücken"
  end
end

class NichtSchwimmer < Mensch
  prepend KannNichtSchwimmen # hänge KannNichtSchwimmen vor NichtSchwimmer ein,
end                          # dh, überdecke die schwimmt-Methode

fisch = Fisch.new
mensch = Mensch.new
vogel = Vogel.new
nicht_schwimmer = NichtSchwimmer.new

fisch.sagt                # => Fisch sagt nichts
vogel.sagt                # => Vogel: Piep
mensch.sagt               # => Mensch: Ich kann mich besser ausdrücken
nicht_schwimmer.sagt      # => NichtSchwimmer: Ich kann mich besser ausdrücken
puts
fisch.schwimmt            # => Fisch schwimmt
vogel.schwimmt            # => Vogel geht unter
mensch.schwimmt           # => Mensch schwimmt
nicht_schwimmer.schwimmt  # => NichtSchwimmer geht unter


class Numeric # ruby-interne Klasse
  def inverse # neue Methode, berechne den Kehrwert
    1.0 / self
  end
end

5.inverse # => 0.2

FactoryBot.define do
  factory :user do
    first_name "Max"
    last_name  "Mustermann"
    admin false
  end
  factory :admin, class: User do
    first_name "Admin"
    last_name  "Root"
    admin true
  end
end



a = "\nThis is a double-quoted string\n"
a = %Q{\nThis is a double-quoted string\n}
a = %{\nThis is a double-quoted string\n}
a = %/\nThis is a double-quoted string\n/
a = <<-BLOCK

This is a double-quoted string
BLOCK

var = 3.14159
"pi is #{var}"

a = 'This is a single-quoted string'
a = %q{This is a single-quoted string}

# Generate a random number and print whether it's even or odd.
if rand(100).even?
  puts "It's even"
else
  puts "It's odd"
end

{ puts 'Hello, World!' } # note the braces
# or:
do
  puts 'Hello, World!'
end

# In an object instance variable (denoted with '@'), remember a block.
def remember(&a_block)
  @block = a_block
end

# Invoke the preceding method, giving it a block that takes a name.
remember {|name| puts "Hello, #{name}!"}

# Call the closure (note that this happens not to close over any free variables):
@block.call('Jon')   # => "Hello, Jon!"

def create_set_and_get(initial_value=0) # note the default value of 0
  closure_value = initial_value
  [ Proc.new {|x| closure_value = x}, Proc.new { closure_value } ]
end

setter, getter = create_set_and_get  # returns two values
setter.call(21)
getter.call      # => 21

# here doc test
puts <<GROCERY_LIST
Grocery list
----
1. Salad mix.
2. Strawberries.*

def dont_show_this(closure_value=0)
[ proc {|x| closure_value = x } , proc { closure_value } ]
end

3. Cereal.
4. Milk.*

* Organic
GROCERY_LIST

# Parameter variables can also be used as a binding for the closure,
# so the preceding can be rewritten as:
def create_set_and_get(closure_value=0)
  [ proc {|x| closure_value = x } , proc { closure_value } ]
end

