# Git configuration (for GitHub)

- Git has to be installed: `sudo apt install git`
- recommended configuration (as commands):
  ```
  git config --global user.name "Max Mustermann"
  git config --global user.email "max.mustermann@email-provider.com"
  git config --global rebase.stat true
  git config --global merge.conflictstyle diff3
  ```
- You need to use a SSH key for GitHub (https://docs.github.com/en/github/authenticating-to-github/connecting-to-github-with-ssh):
  - Create SSH-Key: `ssh-keygen -t ed25519 -C "max.mustermann@email-provider.com"`
  - Use the default path
  - Enter the password you want to use (optional)
  - Copy the public key: `cat ~/.ssh/id_ed25519.pub`
  - Go to GitHub (https://github.com/settings/keys) and add the key (the title is not important)
  - now you can clone the GitHub repositories using the SSH URLs (for example `git clone git@github.com:NicoJG/Repo_Name.git`)
