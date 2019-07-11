      program asgngoto

      integer num, dest

      print*, 'Input num:'
      read*, num

      assign 10 to dest
      if (num .eq. 2) assign 20 to dest
      if (num .eq. 3) assign 30 to dest
      if (num .eq. 4) assign 40 to dest

* Here is the computed goto:
      goto dest, (20, 30, 40)

10    print*, 'Input out of range'
      return
20    print*, 'Two!'
      return
30    print*, 'Three!'
      return
40    print*, 'Four!'
      return
      end
