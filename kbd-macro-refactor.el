(fset 'camelize-tags-auto-kmacro
   (lambda (&optional arg) "Keyboard macro." (interactive "p") (kmacro-exec-ring-item (quote (") {(xtoggle-camel	b fwxtrtags-rename-s	y" 0 "%d")) arg)))

(fset 'camelize-header-rtags-auto-kmacro
   (lambda (&optional arg) "Keyboard macro." (interactive "p") (kmacro-exec-ring-item (quote (");(xtoggle-came	b fwxrtags	rename-sym	y" 0 "%d")) arg)))
