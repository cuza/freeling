
 require 'freeling'

 tk = Freeling::tokenizer.new("/use/local/share/freeling/es/tokenizer.dat")

 while (line=gets.chomp)
   s = tk.tokenize(line)
 end

